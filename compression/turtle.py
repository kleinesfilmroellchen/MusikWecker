from typing import Optional, Self
from enum import Enum
from dataclasses import dataclass, field
from bits import BitStream
import numpy as np
from itertools import product
from PIL import Image

x_size = 80
y_size = 64


# reached a dead end
class DeadendException(Exception):
    pass


# reached a position where we want to move to an already covered pixel -> we’re snaking and must discard this object
class SnakingException(Exception):
    pass


class Command(Enum):
    Left135 = "135L"
    Left90 = "90L"
    Left45 = "45L"
    Forward1 = "F1"
    ForwardN = "FN"
    ForwardMany = "FX"
    Right45 = "45R"
    Right90 = "90R"
    Right135 = "135R"

    def direction_delta(self) -> int:
        match self:
            case Command.Left135:
                return 5
            case Command.Left90:
                return 6
            case Command.Left45:
                return 7
            case Command.Forward1 | Command.ForwardN | Command.ForwardMany:
                return 0
            case Command.Right45:
                return 1
            case Command.Right90:
                return 2
            case Command.Right135:
                return 3


@dataclass(slots=True, kw_only=True, frozen=True, match_args=True)
class HuffmanTreeNode:
    probability: float
    left: Optional[Self] = field(default=None)
    right: Optional[Self] = field(default=None)
    command: Optional[Command] = field(default=None)

    @staticmethod
    def new_inner(left: Self, right: Self) -> Self:
        return HuffmanTreeNode(
            probability=left.probability + right.probability, left=left, right=right
        )

    @staticmethod
    def new_leaf(probability: float, command: Command) -> Self:
        return HuffmanTreeNode(probability=probability, command=command)

    def is_leaf(self):
        return self.command is not None

    def bit_sequence_for(self, command: Command) -> Optional[list[bool]]:
        if self.is_leaf():
            if self.command == command:
                return list()
            else:
                return None
        else:
            left_sequence = self.left.bit_sequence_for(command)
            if left_sequence is not None:
                left_sequence.insert(0, True)
                return left_sequence
            right_sequence = self.right.bit_sequence_for(command)
            if right_sequence is not None:
                right_sequence.insert(0, False)
                return right_sequence
            return None


@dataclass(frozen=False, init=False)
class HuffmanTable:
    code_table: dict[Command, list[bool]]

    def __init__(self, probabilities: dict[Command, float]):
        self.code_table = HuffmanTable.compute_table(probabilities)

    @staticmethod
    def compute_table(probabilities: dict[Command, float]) -> dict[Command, list[bool]]:
        trees: list[HuffmanTreeNode] = []
        for command, probability in probabilities.items():
            trees.append(HuffmanTreeNode.new_leaf(probability, command))

        while len(trees) > 1:
            trees.sort(key=lambda tree: tree.probability)
            lowest = trees.pop(0)
            second_lowest = trees.pop(0)
            trees.append(HuffmanTreeNode.new_inner(lowest, second_lowest))

        tree = trees[0]

        table: dict[Command, list[bool]] = {}
        for command in Command:
            table[command] = tree.bit_sequence_for(command) or []
        return table

    def write_command(self, stream: BitStream, command: Command):
        for bit in self.code_table[command]:
            stream.write_bits(int(bit), 1)


def move_in_direction(direction: int, x: int, y: int) -> tuple[int, int]:
    match direction:
        case 0:
            return (x + 1, y)
        case 1:
            return (x + 1, y + 1)
        case 2:
            return (x, y + 1)
        case 3:
            return (x - 1, y + 1)
        case 4:
            return (x - 1, y)
        case 5:
            return (x - 1, y - 1)
        case 6:
            return (x, y - 1)
        case 7:
            return (x + 1, y - 1)


def direction_delta_to_command(current_direction: int, next_direction: int) -> Command:
    match (next_direction - current_direction) % 8:
        case 5:
            return Command.Left135
        case 6:
            return Command.Left90
        case 7:
            return Command.Left45
        case 0:
            return Command.Forward1
        case 1:
            return Command.Right45
        case 2:
            return Command.Right90
        case 3:
            return Command.Right135
        case _:
            raise Exception(
                f"backwards direction delta between {current_direction} and {next_direction} is illegal"
            )


def neighbor_positions(image: np.array, x: int, y: int) -> list[tuple[int, int]]:
    x_size, y_size = image.shape
    neighbors = []
    for delta_x, delta_y in [
        (+1, 0),
        (+1, +1),
        (0, +1),
        (-1, +1),
        (-1, 0),
        (-1, -1),
        (0, -1),
        (+1, -1),
    ]:
        neighbor_x, neighbor_y = x + delta_x, y + delta_y
        if (
            neighbor_x >= x_size
            or neighbor_x < 0
            or neighbor_y >= y_size
            or neighbor_y < 0
        ):
            continue
        neighbor = image[neighbor_x, neighbor_y]
        if neighbor:
            neighbors.append((neighbor_x, neighbor_y))

    return neighbors


def leftmost_uncovered_neighbor(
    image: np.array,
    covered: np.array,
    deadend: np.array,
    x: int,
    y: int,
    direction: int,
    start_x: int,
    start_y: int,
) -> tuple[int, int, int]:
    """Returns a tuple of (x, y, direction)."""
    legal_neighbors = []
    for neighbor_x, neighbor_y in neighbor_positions(image, x, y):
        if not deadend[neighbor_x, neighbor_y]:
            legal_neighbors.append((neighbor_x, neighbor_y))

    found_covered_pixel = False
    # iterate through directions from the left of the current one and find first matching neighbor
    for direction_delta in range(-3, 4):
        new_direction = (direction + direction_delta) % 8
        new_neighbor_x, new_neighbor_y = move_in_direction(new_direction, x, y)
        # print(f"considering dir {new_direction} -> {new_neighbor}")
        if (new_neighbor_x, new_neighbor_y) in legal_neighbors:
            # found start
            if start_x == new_neighbor_x and start_y == new_neighbor_y:
                return (new_neighbor_x, new_neighbor_y, new_direction)
            # remember for later that there is a covered pixel
            if covered[new_neighbor_x, new_neighbor_y]:
                found_covered_pixel = True
            else:
                # if our leftmost possible position is one of (but not the only) available pixels,
                # we’re snaking in on ourselves, so we cannot reach the start through an entirely new path.
                # if we only have covered pixels available, we’re also in a deadend (handled below, after loop exits unsuccessfully)
                if found_covered_pixel:
                    raise SnakingException(
                        f"trying to move to covered position {x}, {y}, we’re snaking in on ourselves"
                    )
                return (new_neighbor_x, new_neighbor_y, new_direction)
    raise DeadendException(
        f"cannot find uncovered neighbor from position {x}, {y} in direction {direction}"
    )


def undo_command(
    command: tuple[Command, int], x: int, y: int, direction: int
) -> tuple[int, int, int]:
    # backwards from current direction, to return to old location
    back_direction = (direction + 4) % 8
    last_x, last_y = move_in_direction(back_direction, x, y)
    # turn back by the command’s turning amount to return to the old direction
    return last_x, last_y, (direction - command[0].direction_delta()) % 8


def encode_object_from(
    image: np.array, start_x: int, start_y: int
) -> tuple[list[tuple[Command, int]], np.array, np.array]:
    """Returns a list of commands (command, step_distance), the edge pixels, and the deadend pixels"""
    commands: list[tuple[Command, int]] = []

    current_x, current_y = start_x, start_y
    current_direction = 1
    covered_pixels = np.zeros_like(image)
    deadend_pixels = np.zeros_like(image)
    covered_pixels[current_x, current_y] = True

    while True:
        # print(f"    move from {current_x}, {current_y} current dir {current_direction}")
        try:
            # find leftmost uncovered neighbor we can reach with our turn radius
            next_x, next_y, next_direction = leftmost_uncovered_neighbor(
                image,
                covered_pixels,
                deadend_pixels,
                current_x,
                current_y,
                current_direction,
                start_x,
                start_y,
            )
        except DeadendException:
            # cannot move from here: we’re stuck, let’s move back
            prev_x, prev_y, prev_direction = undo_command(
                commands.pop(), current_x, current_y, current_direction
            )
            # print(
            #     f"        got stuck at {current_x}, {current_y}, going back to {prev_x}, {prev_y} dir {prev_direction}"
            # )
            deadend_pixels[current_x, current_y] = True
            current_x, current_y, current_direction = prev_x, prev_y, prev_direction
            continue

        # move to neighbor and insert appropriate command
        command = direction_delta_to_command(current_direction, next_direction)
        commands.append((command, 1))
        # print(f"        {command}")
        current_x, current_y, current_direction = next_x, next_y, next_direction
        # cover new pixel so that we don’t go there again
        covered_pixels[current_x, current_y] = True

        if current_x == start_x and current_y == start_y:
            break

    # todo: consolidate forward movement commands

    return commands, covered_pixels, deadend_pixels


def fill_object(
    start_x: int,
    start_y: int,
    object_commands: list[tuple[Command, int]],
    edge: np.array,
) -> bytes:
    filled = edge.copy()

    # find first pixel to the right of the edge that is not covered
    current_direction = 1
    current_x = start_x
    current_y = start_y
    blank_pixel_x, blank_pixel_y = None, None
    counter = 0
    for command, dist in object_commands:
        current_direction = (current_direction + command.direction_delta()) % 8
        right_direction = (current_direction + 2) % 8
        right_pixel_x, right_pixel_y = move_in_direction(
            right_direction, current_x, current_y
        )
        dx, dy = move_in_direction(current_direction, current_x, current_y)
        current_x, current_y = dx * dist, dy * dist
        counter += 1
        # don’t do this for the start location. later on, we know that any position right of us *must* be covered, because our successor can only move diagonally back right, which is to our right.
        if counter == 1:
            continue
        if not edge[right_pixel_x, right_pixel_y]:
            blank_pixel_x, blank_pixel_y = right_pixel_x, right_pixel_y
            break

    if blank_pixel_x is None and blank_pixel_y is None:
        # nothing to do, edge has no gaps
        return filled
    # print(f"        found right pixel {blank_pixel_x}, {blank_pixel_y}")

    # flood fill algorithm
    unfilled_neighbor_queue: set[tuple[int, int]] = {(blank_pixel_x, blank_pixel_y)}
    while len(unfilled_neighbor_queue) > 0:
        new_x, new_y = unfilled_neighbor_queue.pop()
        # print(f"        filling {new_x}, {new_y}")
        filled[new_x, new_y] = True
        # only consider rectangular neighbors, since edges can span diagonals
        for delta_x, delta_y in [
            (+1, 0),
            (0, +1),
            (-1, 0),
            (0, -1),
        ]:
            next_x, next_y = new_x + delta_x, new_y + delta_y
            if (
                next_x >= x_size or next_x < 0 or next_y >= y_size or next_y < 0
            ) or filled[next_x, next_y]:
                continue
            unfilled_neighbor_queue.add((next_x, next_y))

    # import sys
    # sys.exit()
    return filled


def fill_object_even_odd(
    start_x: int,
    start_y: int,
    object_commands: list[tuple[Command, int]],
    edge: np.array,
) -> bytes:
    filled = edge.copy()

    row_counts = np.zeros(shape=filled.shape[1], dtype=int)
    col_counts = np.zeros(shape=filled.shape[0], dtype=int)

    for y in range(y_size):
        for x in range(x_size):
            if edge[x, y]:
                # if previous pixel was filled too, do not increment count in any direction
                if not (x > 0 and edge[x - 1, y]):
                    row_counts[y] += 1

                if not (y > 0 and edge[x, y - 1]):
                    col_counts[x] += 1
            # even-odd-rule, with “not filled” always winning
            else:
                if row_counts[y] % 2 == 1 and col_counts[x] % 2 == 1:
                    # print(f"fill {x}, {y}, row {row_counts[y]} col {col_counts[x]}")
                    filled[x, y] = True
    # import sys
    # sys.exit()
    return filled


def is_point_in_path(x: int, y: int, poly: list[tuple[int, int]]) -> bool:
    """Determine if the point is on the path, corner, or boundary of the polygon

    Args:
      x -- The x coordinates of point.
      y -- The y coordinates of point.
      poly -- a list of tuples [(x, y), (x, y), ...]

    Returns:
      True if the point is in the path or is a corner or on the boundary"""
    c = False
    for i in range(len(poly)):
        ax, ay = poly[i]
        bx, by = poly[i - 1]
        if (x == ax) and (y == ay):
            # point is a corner
            return True
        if (ay > y) != (by > y):
            slope = (x - ax) * (by - ay) - (bx - ax) * (y - ay)
            if slope == 0:
                # point is on boundary
                return True
            if (slope < 0) != (by < ay):
                c = not c
    return c


def fill_object_poly(
    start_x: int,
    start_y: int,
    object_commands: list[tuple[Command, int]],
    edge: np.array,
) -> bytes:
    filled = edge.copy()

    # find first pixel to the right of the edge that is not covered
    current_direction = 1
    current_x = start_x
    current_y = start_y
    edge_list = [(current_x, current_y)]
    for command, dist in object_commands:
        current_direction = (current_direction + command.direction_delta()) % 8
        dx, dy = move_in_direction(current_direction, current_x, current_y)
        current_x, current_y = dx * dist, dy * dist
        edge_list.append((current_x, current_y))

    for y, x in product(range(y_size), range(x_size)):
        if is_point_in_path(x, y, edge_list):
            filled[x, y] = True

    return filled


def encode_turtle(data: bytes, frame:int) -> bytes:
    output = bytearray()

    image = np.zeros(shape=(x_size, y_size), dtype=bool)
    debug_image: Image.Image = Image.new(mode="RGB", size=(x_size, y_size))

    x, y = 0, 0
    for byte in data:
        for bit in range(8):
            pixel_value = 1 if (byte & (1 << bit)) >= 1 else 0
            image[x, y] = bool(pixel_value)
            x += 1
            if x == x_size:
                x = 0
                y += 1

    # image copy with all currently present objects, will be gradually filled up
    blitted = np.zeros_like(image)
    single_pixels = np.zeros_like(image)
    while not np.array_equal(blitted, image):
        # search for first white pixel in the image that’s not been blitted yet
        # (by searching from top and left, we’ll always find an object edge)
        found_x, found_y = None, None
        for y, x in product(range(y_size), range(x_size)):
            if image[x, y] and not blitted[x, y]:
                found_x, found_y = x, y
                break
        # print(f"start encoding from {found_x}, {found_y}")
        if found_x is None and found_y is None:
            break

        try:
            # encode object starting at that pixel
            object_commands, edge, deadends = encode_object_from(
                image ^ blitted, found_x, found_y
            )
            # fill object edge
            filled = fill_object_poly(found_x, found_y, object_commands, edge)
            for y, x in product(range(y_size), range(x_size)):
                if filled[x, y]:
                    debug_image.putpixel((x, y), (0, 255, 0))
                if edge[x, y]:
                    debug_image.putpixel((x, y), (255, 255, 0))
                if deadends[x, y]:
                    debug_image.putpixel((x, y), (255, 0, 127))
            # copy filled surface to blitted
            # todo: should be xor
            blitted = blitted | filled
            blitted = blitted  | deadends
            single_pixels = single_pixels | deadends
            # todo: store command list
        except Exception as e:
            # print(f"    couldn’t find object here, marking single pixel ({e})")
            # todo: mark deadends instead of start if they were found (they’re more likely the issue)
            single_pixels[found_x, found_y] = True
            blitted[found_x, found_y] = True
            debug_image.putpixel((x, y), (255, 0, 0))

    debug_image.save(f"frames/debug_{frame}.png")

    return bytes(output)


if __name__ == "__main__":
    table = HuffmanTable(
        {
            Command.Forward1: 0.3,
            Command.ForwardN: 0.3,
            Command.ForwardMany: 0.3,
            Command.Left45: 0.1,
            Command.Left90: 0.1,
            Command.Right90: 0.1,
            Command.Right45: 0.1,
        }
    )
    print(table)
    stream = BitStream()
    for command in Command:
        table.write_command(stream, command)
    stream.write_to_byte_boundary()
    print([bin(e) for e in stream.data])

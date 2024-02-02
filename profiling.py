import os
import statistics

from rich.console import Console
from rich.table import Table


class Intern:
    """
    input file format:
    ```
    ...: Unix timestamp
    ...: Unix timestamp
    .....
    ....
    ...
    ..
    .
    ```
    + seq is `: `(have space)
    + no unit in tail(default unit is `us`)
    + ignore endl
    """

    @staticmethod
    def summary(costs: list[float]) -> str:
        if len(costs) == 0:
            return ""
        elif len(costs) == 1:
            return f"{costs[0]:.2f}"
        else:
            mean = statistics.mean(costs)
            stdev = statistics.stdev(costs)
            return f"{mean:.2f} ± {stdev:.2f}"

    @staticmethod
    def print_table(
        data: list[tuple[str, ...]], headers: None | list[str] | tuple[str, ...] = None
    ):
        if len(data):
            assert all(len(row) == len(data[0]) for row in data)
            if headers is not None:
                assert len(headers) == len(data[0])

        console = Console()
        table = Table()
        if headers is None:
            table = Table(show_header=False)
        else:
            for header in headers:
                table.add_column(header)

        for row in data:
            table.add_row(*row)

        console.print(table)

    @staticmethod
    def rate(unit: str) -> float:
        transfer = {
            "ns": 1,
            "us": 1000,
            "ms": 1000 * 1000,
            "s": 1000 * 1000 * 1000,
        }
        assert unit in transfer, unit + " is not a valid unit"
        return transfer[unit]

    def __init__(self) -> None:
        self.global_result: list[tuple[str, ...]] = []

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if exc_type is None:
            Intern.print_table(self.global_result, ["example", "time", "number"])

    def append(self, name: str, costs: list[float | None], unit: str):
        real_costs: list[float] = [cost for cost in costs if cost is not None]

        self.global_result.append(
            (name, Intern.summary(real_costs) + " " + unit, str(len(real_costs)))
        )

    def benchmark(self, input_filepath: str, example_name: str, dst_unit: str):
        assert os.path.exists(input_filepath)
        rate = Intern.rate(dst_unit)
        costs: list[float | None] = []
        with open(input_filepath, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if len(line) == 0:
                    costs.append(None)
                    continue
                cost = float(line.split(": ")[1])
                cost /= rate
                costs.append(cost)
        self.append(example_name, costs, dst_unit)

    def trace(
        self,
        input_filepath: str,
        example_name: str,
        points: list[str],
        dst_unit: str,
        graph: bool = False,
    ):
        assert os.path.exists(input_filepath)
        assert len(points) >= 2
        rate = Intern.rate(dst_unit)
        costs: dict[str, list[float | None]] = {}
        for i in range(len(points) - 1):
            stage = points[i] + " to " + points[i + 1]
            costs[stage] = []
        sum_costs: list[float | None] = []
        with open(input_filepath, "r", encoding="utf-8") as f:
            while True:
                group: list[float] = []

                def consume_line_from_str(line: str):
                    line_name, line_value_str = line.split(": ")
                    assert line_name == points[len(group)]
                    line_value = int(line_value_str)
                    if line_value == 0:
                        exit()
                    group.append(line_value)

                def consume_line_from_iter():
                    line = next(f).strip()
                    consume_line_from_str(line)

                try:
                    line = next(f).strip()
                    if len(line) == 0:
                        continue
                    consume_line_from_str(line)
                    for _ in range(len(points) - 1):
                        consume_line_from_iter()

                    assert len(group) == len(points)
                    for i in range(len(points) - 1):
                        stage = points[i] + " to " + points[i + 1]
                        cost = group[i + 1] - group[i]
                        if cost <= 0:
                            costs[stage].append(None)
                            continue
                        cost /= rate
                        costs[stage].append(cost)

                    sum_cost = group[-1] - group[0]
                    if sum_cost > 0:
                        sum_costs.append(sum_cost / rate)
                    else:
                        sum_costs.append(None)

                except StopIteration:
                    break

        self.append(example_name, sum_costs, dst_unit)
        for k, v in costs.items():
            self.append("  " + k, v, dst_unit)

        if graph:
            import matplotlib.pyplot as plt
            import numpy as np

            assert example_name not in costs
            costs[example_name] = sum_costs

            length = max(len(cost) for cost in costs.values())
            assert all(len(cost) == length for cost in costs.values())

            x = np.arange(length)

            fig, ax = plt.subplots()

            for k, v in costs.items():
                assert len(x) == len(v)
                # ax.plot(x, v, label=k)
                ax.plot(x, v)

            ax.legend()

            ax.set_ylabel(dst_unit)

            plt.savefig(input_filepath + ".png")


with Intern() as intern:
    intern.benchmark("pingpong_sleep.txt", "pingpong sleep", "ms")
    intern.benchmark("pingpong_nosleep.txt", "pingpong nosleep", "ms")
    intern.trace(
        "pingpong_sleep_trace.txt",
        "ping pong sleep trace",
        [
            "read_start",
            "read_end",
            "write_start",
            "write_end",
        ],
        "us",
    )
    intern.trace(
        "pingpong_nosleep_trace.txt",
        "ping pong nosleep trace",
        [
            "read_start",
            "read_end",
            "write_start",
            "write_end",
        ],
        "us",
    )

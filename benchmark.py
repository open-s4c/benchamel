# Copyright (C) 2025 Huawei Technologies Co., Ltd. All rights reserved.

import pathlib
from typing import Any, Dict, List

from benchkit.benchmark import Benchmark
from benchkit.utils.dir import get_curdir


class Benchamel(Benchmark):

    def __init__(self, ) -> None:
        super().__init__(
            command_wrappers=(),
            command_attachments=(),
            shared_libs=(),
            pre_run_hooks=(),
            post_run_hooks=(),
        )
        script_path = get_curdir(__file__)
        self._bench_src_path = script_path
        self._build_dir = script_path / "build"

    @property
    def bench_src_path(self) -> pathlib.Path:
        return self._bench_src_path

    @staticmethod
    def get_build_var_names() -> List[str]:
        return [
            'test',
        ]

    @staticmethod
    def get_run_var_names() -> List[str]:
        return ['test']

    @staticmethod
    def get_tilt_var_names() -> List[str]:
        return []

    def prebuild_bench(  # pylint: disable=arguments-differ
        self,
        benchmark_duration_seconds: int,
    ) -> None:
        pass

    def build_bench(  # pylint: disable=arguments-differ
        self,
        test: str,
        benchmark_duration_seconds: int,
        **_kwargs,
    ) -> None:
        pass

    def clean_bench(self) -> None:
        pass

    def single_run(
        self,
        test: str,
        **kwargs,
    ) -> str:
        run_command = [test]
        output = self.run_bench_command(
            run_command=run_command,
            wrapped_run_command=run_command,
            current_dir=self._build_dir,
            environment=None,
            wrapped_environment=None,
            print_output=True,
        )
        return output

    def parse_output_to_results(  # pylint: disable=arguments-differ
        self,
        command_output: str,
        run_variables: Dict[str, Any],
        **kwargs,
    ) -> Dict[str, Any]:
        return {}

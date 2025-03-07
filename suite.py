#!/usr/bin/env python3
# Copyright (C) 2025 Huawei Technologies Co., Ltd. All rights reserved.

from pathlib import Path
import re
import os
import sys
from itertools import chain

from benchkit.campaign import CampaignCartesianProduct, CampaignSuite
from benchkit.utils.dir import get_curdir
from benchmark import Benchamel

NB_RUNS = 1
TESTS = [
    str(f) for f in chain.from_iterable(
        (get_curdir(__file__) / 'build' / d).rglob('*')
        for d in ['ConVul', 'libvsync', 'sctbench'])
    if os.path.isfile(f) and os.access(f, os.X_OK) and not any(
        str(f).endswith(t)
        for t in ['2017-6346', '2015-7550', 'clhlock', 'bounded_spsc'])
]

DURATION_SECONDS = 10


def main() -> None:
    global TESTS
    if (len(sys.argv) > 1):
        TESTS = list(filter(lambda s: re.search(sys.argv[1], s), TESTS))
    if (len(sys.argv) > 2):
        TESTS = list(filter(lambda s: not re.search(sys.argv[2], s), TESTS))
    campaign = CampaignCartesianProduct(
        name='benchamel',
        benchmark=Benchamel(),
        nb_runs=NB_RUNS,
        variables={'test': TESTS},
        constants={},
        debug=False,
        gdb=False,
        enable_data_dir=False,
        benchmark_duration_seconds=DURATION_SECONDS,
        pretty={'test': {
            s: str(Path(s).stem)
            for s in TESTS
        }},
    )
    campaigns = [campaign]
    suite = CampaignSuite(campaigns=campaigns)
    suite.print_durations()
    suite.run_suite()


if __name__ == '__main__':
    main()

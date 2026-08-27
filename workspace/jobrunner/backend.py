from __future__ import annotations

from .models import JobRecord


class MemoryJobBackend:
    """持久数据库状态的进程内替身。

    测试中，后端对象可以比仓库和服务包装器存活更久。
    生产适配器应把这些状态映射到数据库行和索引。
    """

    def __init__(self) -> None:
        self.jobs: dict[str, JobRecord] = {}

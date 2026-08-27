from __future__ import annotations

from .models import JobRecord


class DispatchError(RuntimeError):
    pass


class FakeDispatcher:
    """外部队列的确定性测试替身。"""

    def __init__(self, failures_before_success: int = 0) -> None:
        self.failures_remaining = failures_before_success
        self.attempted_job_ids: list[str] = []
        self.dispatched_job_ids: list[str] = []

    def dispatch(self, job: JobRecord) -> None:
        self.attempted_job_ids.append(job.id)
        if self.failures_remaining > 0:
            self.failures_remaining -= 1
            raise DispatchError("队列暂时不可用")
        self.dispatched_job_ids.append(job.id)

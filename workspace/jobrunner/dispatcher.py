from __future__ import annotations

from .models import JobRecord


class DispatchError(RuntimeError):
    pass


class FakeDispatcher:
    """Deterministic test double for the external queue."""

    def __init__(self, failures_before_success: int = 0) -> None:
        self.failures_remaining = failures_before_success
        self.attempted_job_ids: list[str] = []
        self.dispatched_job_ids: list[str] = []

    def dispatch(self, job: JobRecord) -> None:
        self.attempted_job_ids.append(job.id)
        if self.failures_remaining > 0:
            self.failures_remaining -= 1
            raise DispatchError("queue temporarily unavailable")
        self.dispatched_job_ids.append(job.id)

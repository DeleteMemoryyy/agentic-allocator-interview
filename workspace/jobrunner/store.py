from __future__ import annotations

from .backend import MemoryJobBackend
from .models import JobRecord


class JobStore:
    """In-memory stand-in for the repository that survives service recreation."""

    def __init__(self, backend: MemoryJobBackend | None = None) -> None:
        self._backend = backend or MemoryJobBackend()

    def insert(self, job: JobRecord) -> None:
        if job.id in self._backend.jobs:
            raise ValueError(f"duplicate job id: {job.id}")
        self._backend.jobs[job.id] = job

    def get(self, job_id: str) -> JobRecord:
        return self._backend.jobs[job_id]

    def mark_dispatched(self, job_id: str) -> None:
        self._backend.jobs[job_id].dispatched = True

    def all_jobs(self) -> tuple[JobRecord, ...]:
        return tuple(self._backend.jobs.values())

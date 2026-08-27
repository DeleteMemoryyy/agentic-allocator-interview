from __future__ import annotations

from .models import JobRecord


class MemoryJobBackend:
    """Process-local stand-in for durable database state.

    The backend object can outlive repository and service wrappers during tests.
    Production adapters would map this state to database rows and indexes.
    """

    def __init__(self) -> None:
        self.jobs: dict[str, JobRecord] = {}

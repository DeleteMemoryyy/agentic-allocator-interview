from __future__ import annotations

from uuid import uuid4

from .dispatcher import FakeDispatcher
from .models import JobRecord
from .store import JobStore


class JobService:
    def __init__(self, store: JobStore, dispatcher: FakeDispatcher) -> None:
        self._store = store
        self._dispatcher = dispatcher
        self._idempotency: dict[str, str] = {}

    def submit(self, tenant_id: str, idempotency_key: str, payload: str) -> JobRecord:
        existing_id = self._idempotency.get(idempotency_key)
        if existing_id is not None:
            return self._store.get(existing_id)

        job = JobRecord(
            id=str(uuid4()),
            tenant_id=tenant_id,
            idempotency_key=idempotency_key,
            payload=payload,
        )
        self._store.insert(job)
        self._idempotency[idempotency_key] = job.id
        self._dispatcher.dispatch(job)
        self._store.mark_dispatched(job.id)
        return job

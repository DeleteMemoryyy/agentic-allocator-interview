from __future__ import annotations

from dataclasses import dataclass


@dataclass
class JobRecord:
    id: str
    tenant_id: str
    idempotency_key: str
    payload: str
    dispatched: bool = False

# Domain Context

## Glossary

### Logical Request
A caller operation identified by `(tenant_id, idempotency_key)` and one payload.

### Admission Record
The durable `JobRecord` representing one Logical Request.

### Dispatch Attempt
One call across the dispatcher boundary that may or may not create an external side effect.

### Definitive Rejection
An outcome proving that a Dispatch Attempt produced no external side effect.

### Ambiguous Outcome
An outcome where the caller cannot prove whether the external system accepted the Dispatch Attempt.

### Delivery Token
A stable identity that an idempotent downstream can use to collapse repeated Dispatch Attempts into one external effect.

### Reconciliation Evidence
An authoritative observation that resolves an Ambiguous Outcome as accepted or not accepted.

### Delivery Profile
The release-level protocol selected for recovering from an Ambiguous Outcome.

### Public Seam
An Interface shared with an adjacent owner. This task has a caller-facing service seam and a dispatcher-facing integration seam.

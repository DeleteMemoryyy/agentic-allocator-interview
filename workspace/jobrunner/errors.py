class IdempotencyConflict(ValueError):
    """The same scoped key was reused for a different logical request."""


class DispatchUnavailable(RuntimeError):
    """The job is durable but could not be dispatched during this attempt."""

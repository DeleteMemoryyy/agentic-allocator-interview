class IdempotencyConflict(ValueError):
    """同一范围内的 key 被用于不同的逻辑请求。"""


class DispatchUnavailable(RuntimeError):
    """任务已持久化，但本次尝试未能派发。"""

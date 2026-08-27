from __future__ import annotations

import unittest

from jobrunner import FakeDispatcher, JobService, JobStore


class PublicJobAdmissionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.store = JobStore()
        self.dispatcher = FakeDispatcher()
        self.service = JobService(self.store, self.dispatcher)

    def test_new_submission_is_stored_and_dispatched(self) -> None:
        job = self.service.submit("tenant-a", "request-1", "compile")
        self.assertEqual([job.id], self.dispatcher.dispatched_job_ids)
        self.assertTrue(self.store.get(job.id).dispatched)

    def test_duplicate_on_same_service_returns_existing_job(self) -> None:
        first = self.service.submit("tenant-a", "request-2", "test")
        second = self.service.submit("tenant-a", "request-2", "test")
        self.assertEqual(first.id, second.id)
        self.assertEqual([first.id], self.dispatcher.dispatched_job_ids)
        self.assertEqual(1, len(self.store.all_jobs()))

    def test_distinct_keys_create_distinct_jobs(self) -> None:
        first = self.service.submit("tenant-a", "request-3", "build")
        second = self.service.submit("tenant-a", "request-4", "deploy")
        self.assertNotEqual(first.id, second.id)
        self.assertEqual(2, len(self.store.all_jobs()))


if __name__ == "__main__":
    unittest.main()

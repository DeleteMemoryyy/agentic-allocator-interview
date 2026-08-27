# Agentic Allocator Interview — Candidate Kit

A 60-minute, agent-enabled systems interview based on allocator incident response rather than implementing `malloc` from scratch.

The starter passes every public test but contains at least one latent production bug. Candidates may use up to two asynchronous coding agents, while remaining responsible for reviewing the final diff, constructing an independent counterexample, and correcting assumptions after private-verifier feedback.

## Candidate task

Read [TASK.md](TASK.md). Submit only:

- `allocator.c`
- `candidate.trace`
- `decision.md`

Run the public verifier with:

```bash
./verify-public.sh
```

Requirements: a C11 compiler (`clang` by default), AddressSanitizer and UndefinedBehaviorSanitizer.

## Interview format

- 0–5 min: model allocator invariants.
- 5–25 min: asynchronous Agent work, review and Phase A submission.
- 25–28 min: private verifier returns only a failure category.
- 28–43 min: correction and Final submission.
- 47–60 min: human review of system model, Agent decisions, evidence and ownership.

The private judge, hidden traces, reference solution and interviewer answer key are intentionally not included in this public candidate repository.

## Status

This is an interview prototype, not a production memory allocator. Do not use it as a general-purpose allocator.

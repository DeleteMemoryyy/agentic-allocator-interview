# Prompt for the Delivery Handoff

Inspect the actual submitted workspace, the task contract, every stage update you received, and the latest verification results. Then replace `HANDOFF.md` with an accurate delivery handoff using its existing headings.

Rules:

- State whether the delivery is ready to merge; do not assume that passing public tests is sufficient.
- Read `submission.json.deliveryProfile`; name that exact profile and keep every claim consistent with it.
- Explain which Public Seam changed, which profile was rejected, and the operational cost of the selected guarantee.
- Describe only behavior present in the submitted snapshot.
- Tie every positive claim to a reproducible test or command.
- If common or selected-profile behavior is incomplete, say so explicitly and describe its impact. Do not claim both profiles merely because helpers for both exist.
- Distinguish an unverified risk from a known failure.
- Give the smallest safe rollback, not a generic statement such as "revert if needed".
- Do not include Agent transcripts, prompts, self-evaluation, or invented Phase feedback.
- Keep the document at or below 800 whitespace-delimited words and 8,000 characters.

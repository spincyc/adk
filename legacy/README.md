# Legacy preview

This directory preserves ADK’s original `0.0.1` preview for historical lessons.
It is frozen, unsupported, and excluded from normal builds and releases.

Use the root library for the first-class RAII interfaces. Legacy code retains
the global object registry and incomplete shutdown behavior documented in its
lessons; do not use it as the basis for new components.

Run `make legacy-check` from the repository root to verify the archive.

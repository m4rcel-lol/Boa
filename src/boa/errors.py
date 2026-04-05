"""Custom errors for Boa transpilation."""

from __future__ import annotations


class BoaError(Exception):
    """Base exception type for Boa tooling."""


class BoaSyntaxError(BoaError):
    """Raised when Boa source cannot be transpiled."""

    def __init__(self, file_name: str, line_number: int, line_text: str, message: str) -> None:
        """Initialize a syntax error with precise source context."""
        self.file_name = file_name
        self.line_number = line_number
        self.line_text = line_text
        self.message = message
        super().__init__(self.format_message())

    def format_message(self) -> str:
        """Return a human-readable error message with location context."""
        return (
            f"{self.file_name}:{self.line_number}: {self.message}\n"
            f"  >> {self.line_text.rstrip()}"
        )

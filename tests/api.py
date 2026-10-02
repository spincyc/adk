"""Check that header comments cannot swallow public API declarations.

Run with python3 tests/api.py. No generated files are written.
"""

import sys
import unittest
from pathlib import Path

ROOT = Path (__file__).resolve ().parents[1]
sys.dont_write_bytecode = True
sys.path.insert (0, str (ROOT / "docs" / "_theme"))

import api  # noqa: E402


class ApiTests (unittest.TestCase):
    def test_lora_settings (self):
        text = api.document (ROOT / "src" / "adk" / "lora_modem.h", "LoraSettings")
        self.assertIn ("LoraSpeed speed = LoraSpeed::Far;\n", text)
        fields = [
            ("uint16_t partner = 0", "where send (text) goes; 0 for every modem"),
            ("uint8_t power = 15", "dBm, from 0 to 15; 14 at most in Europe"),
            ("uint8_t network = 6", "1 to 15: modems hear only their own"),
            ("uint32_t band = 915000000", "Hz: 915 MHz, for the USA and Canada"),
        ]
        rows = [line for line in text.splitlines () if line.startswith ("| `")]
        self.assertEqual (rows, [f"| `{signature}` | {note} |" for signature, note in fields])
        self.assertNotIn ("//", text)

    def test_trailing_note_belongs_only_to_its_declaration (self):
        groups = api.declarations ([
            "// Destination settings.",
            "uint16_t partner = 0; // Zero broadcasts; send (text) uses this.",
            "uint8_t speed = 1;",
            "uint8_t power = 15; // 0 to 15.",
            "// Read either counter.",
            "int sent () const;",
            "int received () const;",
        ])
        self.assertEqual (groups, [
            [["uint16_t partner = 0"],
             "Destination settings. Zero broadcasts; send (text) uses this."],
            [["uint8_t speed = 1"], ""],
            [["uint8_t power = 15"], "0 to 15."],
            [["int sent () const", "int received () const"], "Read either counter."],
        ])

    def test_multiline_declaration (self):
        groups = api.declarations ([
            "// Read a range.",
            "int read (int first,",
            "          int last = 9); // Inclusive endpoints.",
            "void clear ();",
        ])
        self.assertEqual (groups, [
            [["int read (int first, int last = 9)"], "Read a range. Inclusive endpoints."],
            [["void clear ()"], ""],
        ])

    def test_comment_markers_in_literals (self):
        source = r'const char* url = "https://example.test/\"quoted\""; // Default URL.'
        self.assertEqual (api.declarations ([source, "char slash = '/'; // Separator."]), [
            [[r'const char* url = "https://example.test/\"quoted\""'], "Default URL."],
            [["char slash = '/'"], "Separator."],
        ])

    def test_body_and_comment_braces (self):
        text = api.document_struct ([
            "struct Sample",
            "{",
            "    int count () const { return 4; } // Size, not a block {.",
            "    int total () const",
            "    { // Body { is not nested.",
            "        return 8; // Nor is this } a close.",
            "    }",
            "    int next = 2; // Initial count.",
            "  private:",
            "    int hidden;",
            "};",
        ], "Sample", "fixture.h")
        self.assertIn ("| `int count () const` | Size, not a block {. |", text)
        self.assertIn ("| `int total () const` | Body { is not nested. |", text)
        self.assertIn ("| `int next = 2` | Initial count. |", text)
        self.assertNotIn ("return", text)
        self.assertNotIn ("hidden", text)

    def test_enum_comments_do_not_hide_following_calls (self):
        groups = api.declarations ([
            "enum class Mode",
            "{",
            "    Slow, // Slow uses less power.",
            "    Fast  // Fast finishes sooner.",
            "};",
            "void start (); // Begin work.",
        ])
        self.assertEqual (groups, [
            [["enum class Mode {Slow, Fast}"], "Slow uses less power. Fast finishes sooner."],
            [["void start ()"], "Begin work."],
        ])

    def test_free_function_after_struct (self):
        text = api.document_functions ([
            "struct Sample // A closing brace } in a note.",
            "{",
            "    int hidden;",
            "};",
            "int read (); // The latest value.",
            "void reset ();",
        ])
        self.assertIn ("| `int read ()` | The latest value. |", text)
        self.assertIn ("void reset ();", text)
        self.assertNotIn ("hidden", text)

    def test_constraint_is_not_a_function_body (self):
        groups = api.declarations ([
            "void show (auto number)",
            "    requires requires { number / 2; }",
            "{",
            "    consume (number);",
            "}",
            "void read (auto number) requires requires { number / 2; }; // Number only.",
            "void clear ();",
        ])
        self.assertEqual (groups, [
            [["void show (auto number) requires requires { number / 2; }"], ""],
            [["void read (auto number) requires requires { number / 2; }"], "Number only."],
            [["void clear ()"], ""],
        ])


if __name__ == "__main__":
    unittest.main ()

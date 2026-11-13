"""Regression checks for complete, non-duplicated diagnostic summaries."""
import pathlib
import subprocess
import sys
import tempfile
import unittest


class DiagnosticReportTests(unittest.TestCase):
    def test_all_categories_includes_sql_and_warning_without_duplicate_core_error(self):
        with tempfile.TemporaryDirectory() as directory:
            logs = pathlib.Path(directory)
            prefix = '2026-11-04_17:00:00 '
            core = prefix + 'ERROR [maps] Failed loading map 571\n'
            (logs / 'Errors.txt').write_text(
                prefix + "ERROR [sql.sql] ScriptName 'old_one' exists in database, but no core script found!\n" +
                prefix + "ERROR [sql.sql] ScriptName 'old_two' exists in database, but no core script found!\n" + core)
            (logs / 'Server.log').write_text(
                prefix + 'INFO [server] Ready\n' + core +
                prefix + 'WARN [scripts] Invalid hook 123\n' +
                prefix + 'WARN [scripts] Invalid hook 456\n')
            result = subprocess.run(
                [sys.executable, str(pathlib.Path(__file__).with_name('server_log_report.py')),
                 '--logs', str(logs), '--scope', 'current', '--source', 'all'],
                text=True, capture_output=True, check=True)
            report = next((logs / 'diagnostics').glob('error-summary-*.txt')).read_text()
            self.assertIn('5 diagnostic messages.', report)
            self.assertIn("       2  [sql.sql] ScriptName '<name>'", report)
            self.assertIn('       2  [scripts] Invalid hook #', report)
            self.assertIn('       1  [maps] Failed loading map #', report)
            self.assertNotIn('[server] Ready', report)
            self.assertIn('old_one', report)  # A real example survives grouping.


if __name__ == '__main__':
    unittest.main()

import unittest
from unittest.mock import Mock,patch
from starter_campaign import logout_when_ready


class LogoutTests(unittest.TestCase):
    def test_combat_refusal_retries_until_acknowledged(self):
        client=Mock();client.last_logout_result=1
        client.logout.side_effect=[False,True]
        self.assertTrue(logout_when_ready(client))
        self.assertEqual(client.logout.call_count,2)
        client.attack_stop.assert_called_once()
        client.pump.assert_called_once()

    def test_falling_is_not_retried_or_granted_success(self):
        client=Mock();client.last_logout_result=3;client.logout.return_value=False
        self.assertFalse(logout_when_ready(client))
        client.pump.assert_not_called()

    def test_combat_wait_has_a_deadline(self):
        client=Mock();client.last_logout_result=1;client.logout.return_value=False
        with patch('starter_campaign.time.monotonic',side_effect=[0,46]):
            self.assertFalse(logout_when_ready(client))
        client.pump.assert_not_called()


if __name__=='__main__':unittest.main()

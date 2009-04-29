from askap import get_config

def test_get_config():
    cfg = get_config("askap.logging", "logging_server.ice_cfg")
    assert cfg.endswith("askap/logging/config/logging_server.ice_cfg")

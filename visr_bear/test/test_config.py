import visr_bear
import pytest


def test_config_data_error():
    c = visr_bear.api.Config()
    c.period_size = 512

    with pytest.raises(ValueError, match="data path must be set"):
        c.validate()


def test_config_period_error():
    c = visr_bear.api.Config()
    c.data_path = "foo.tf"

    with pytest.raises(ValueError, match="period size must be set"):
        c.validate()


def test_config_valid():
    c = visr_bear.api.Config()
    c.data_path = "foo.tf"
    c.period_size = 512

    c.validate()

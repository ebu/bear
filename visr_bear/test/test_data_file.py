from contextlib import contextmanager
import pytest
import visr_bear.api
from utils import data_path
import bear.tensorfile


@contextmanager
def mod_data(in_file, out_file):
    with open(in_file, "rb") as f:
        data = bear.tensorfile.read(f)

    yield data

    with open(out_file, "wb") as f:
        bear.tensorfile.write(f, data)


def test_no_metadata(tmp_path):
    data_path_mod = str(tmp_path / "default_no_meta.tf")

    with mod_data(data_path, data_path_mod) as data:
        if "metadata" in data:
            del data["metadata"]

    meta = visr_bear.api.DataFileMetadata.read_from_file(data_path_mod)

    assert not meta.has_metadata
    assert meta.label == ""
    assert not meta.is_released


def test_with_metadata(tmp_path):
    data_path_mod = str(tmp_path / "default_meta.tf")

    with mod_data(data_path, data_path_mod) as data:
        data["metadata"] = dict(
            label="label with ünicode",
            released=True,
        )

    meta = visr_bear.api.DataFileMetadata.read_from_file(data_path_mod)

    assert meta.has_metadata
    assert meta.label == "label with ünicode"
    assert meta.description == ""
    assert meta.is_released


def test_with_description(tmp_path):
    data_path_mod = str(tmp_path / "default_description.tf")

    with mod_data(data_path, data_path_mod) as data:
        data["metadata"] = dict(
            label="the label",
            description="the description",
            released=False,
        )

    meta = visr_bear.api.DataFileMetadata.read_from_file(data_path_mod)

    assert meta.has_metadata
    assert meta.label == "the label"
    assert meta.description == "the description"
    assert not meta.is_released


@pytest.mark.parametrize("version", [None, 0])
def test_version_ok(tmp_path, version):
    data_path_mod = str(tmp_path / f"default_version_{version}.tf")

    with mod_data(data_path, data_path_mod) as data:
        version_attr = "bear_data_version"
        if version is None:
            if version_attr in data:
                del data[version_attr]
        else:
            data[version_attr] = version

    # checks version
    visr_bear.api.DataFileMetadata.read_from_file(data_path_mod)


def test_version_error(tmp_path):
    data_path_mod = str(tmp_path / f"default_version_error.tf")

    with mod_data(data_path, data_path_mod) as data:
        version_attr = "bear_data_version"
        data[version_attr] = 1

    with pytest.raises(
        RuntimeError, match="data file version mismatch: expected 0, got 1"
    ):
        visr_bear.api.DataFileMetadata.read_from_file(data_path_mod)

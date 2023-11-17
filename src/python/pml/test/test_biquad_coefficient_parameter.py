
import pytest

import pml
import rbbl

import numpy as np


def biquadCoefficientParameter( dtype ):
    if dtype == np.float32:
        return pml.BiquadCoefficientParameterFloat
    elif dtype == np.float64:
        return pml.BiquadCoefficientParameterDouble
    else:
        raise ValueError("Unsupported numeric type")


def biquadCoefficient( dtype ):
    if dtype == np.float32:
        return rbbl.BiquadCoefficientFloat
    elif dtype == np.float64:
        return rbbl.BiquadCoefficientDouble
    else:
        raise ValueError("Unsupported numeric type")


def biquadCoefficientListParameter( dtype ):
    if dtype == np.float32:
        return pml.BiquadCoefficientListParameterFloat
    elif dtype == np.float64:
        return pml.BiquadCoefficientListParameterDouble
    else:
        raise ValueError("Unsupported numeric type")


def biquadCoefficientList( dtype ):
    if dtype == np.float32:
        return rbbl.BiquadCoefficientListFloat
    elif dtype == np.float64:
        return rbbl.BiquadCoefficientListDouble
    else:
        raise ValueError("Unsupported numeric type")


def biquadCoefficientMatrixParameter( dtype ):
    if dtype == np.float32:
        return pml.BiquadCoefficientMatrixParameterFloat
    elif dtype == np.float64:
        return pml.BiquadCoefficientMatrixParameterDouble
    else:
        raise ValueError("Unsupported numeric type")


def biquadCoefficientMatrix( dtype ):
    if dtype == np.float32:
        return rbbl.BiquadCoefficientMatrixFloat
    elif dtype == np.float64:
        return rbbl.BiquadCoefficientMatrixDouble
    else:
        raise ValueError("Unsupported numeric type")


# Coefficient data types used in all unit tests.
dtypes = [np.float32, np.float64]

#%% ========================================================================

@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_default_init(dtype):
    bcp = biquadCoefficientParameter(dtype)()
    assert bcp.a1 == 0.0
    assert bcp.a2 == 0.0
    assert bcp.b0 == 1.0
    assert bcp.b1 == 0.0
    assert bcp.b2 == 0.0


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_list_init(dtype):
    b0, b1, b2, a1, a2 = 1.0, -2.0, 3.0, -4.0, 5.0
    bcp = biquadCoefficientParameter(dtype)([b0, b1, b2, a1, a2])
    assert bcp.a1 == a1
    assert bcp.a2 == a2
    assert bcp.b0 == b0
    assert bcp.b1 == b1
    assert bcp.b2 == b2
    

@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_list_init_wrong_len_fail(dtype):
    b0, b1, b2, a1, a2 = 1.0, -2.0, 3.0, -4.0, 5.0
    extra = 3.5
    # Error is detected by pybind11's overloading mechanism & therefore raised as a TypeError.
    with pytest.raises(TypeError):
        bcp = biquadCoefficientParameter(dtype)([b0, b1, b2, a1, a2, extra])


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_scalar_init(dtype):
    b0, b1, b2, a1, a2 = 1.0, -2.0, 3.0, -4.0, 5.0
    bcp = biquadCoefficientParameter(dtype)(b0, b1, b2, a1, a2)
    assert bcp.a1 == a1
    assert bcp.a2 == a2
    assert bcp.b0 == b0
    assert bcp.b1 == b1
    assert bcp.b2 == b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_scalar_init_kwargs(dtype):
    b0, b1, b2, a1, a2 = 1.0, -2.0, 3.0, -4.0, 5.0
    bcp = biquadCoefficientParameter(dtype)(a1=a1, a2=a2, b0=b0, b1=b1, b2=b2)
    assert bcp.a1 == a1
    assert bcp.a2 == a2
    assert bcp.b0 == b0
    assert bcp.b1 == b1
    assert bcp.b2 == b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_init_from_rbbl(dtype):
    b0, b1, b2, a1, a2 = 1.0, -2.0, 3.0, -4.0, 5.0
    bc = biquadCoefficient(dtype)([b0, b1, b2, a1, a2])
    bcp = biquadCoefficientParameter(dtype)(bc)
    assert bcp.a1 == a1
    assert bcp.a2 == a2
    assert bcp.b0 == b0
    assert bcp.b1 == b1
    assert bcp.b2 == b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_init_from_config(dtype):
    cfg = pml.EmptyParameterConfig()
    bcp = biquadCoefficientParameter(dtype)(cfg)
    default = biquadCoefficient(dtype)()
    assert bcp.a1 == default.a1
    assert bcp.a2 == default.a2
    assert bcp.b0 == default.b0
    assert bcp.b1 == default.b1
    assert bcp.b2 == default.b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_parameter_from_wrong_config_fail(dtype):
    cfg = pml.VectorParameterConfig(7)
    with pytest.raises(ValueError):
        bcmp = biquadCoefficientParameter(dtype)(cfg)

#%% ========================================================================

@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_list_parameter_default_init(dtype):
    bclp = biquadCoefficientListParameter(dtype)()
    assert bclp.size == 0

@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_list_parameter_size_init(dtype):
    sz = 7
    bclp = biquadCoefficientListParameter(dtype)(sz)
    assert bclp.size == sz
    default = biquadCoefficient(dtype)()
    for idx in range(sz):
        bcp = bclp[idx]
        assert bcp.a1 == default.a1
        assert bcp.a2 == default.a2
        assert bcp.b0 == default.b0
        assert bcp.b1 == default.b1
        assert bcp.b2 == default.b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_list_parameter_init_from_list(dtype):
    sz = 5
    bcl = []
    for idx in range(sz):
        bc = biquadCoefficient(dtype)(idx + 0.1, idx - 0.2, idx + 0.3, idx - 0.4, idx + 0.5)
        bcl.append(bc)
    assert len(bcl) == sz
    bclp = biquadCoefficientListParameter(dtype)(bcl)
    assert bclp.size == sz
    for idx in range(sz):
        bcp = bclp[idx]
        assert bcp.a1 == bcl[idx].a1
        assert bcp.a2 == bcl[idx].a2
        assert bcp.b0 == bcl[idx].b0
        assert bcp.b1 == bcl[idx].b1
        assert bcp.b2 == bcl[idx].b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_list_parameter_init_from_rbbl(dtype):
    sz = 8
    bcl = biquadCoefficientList(dtype)(sz)
    assert len(bcl) == sz
    for idx in range(sz):
        bcl[idx].a1 = idx + 0.1
        bcl[idx].a2 = idx - 0.2
        bcl[idx].b0 = idx + 0.3
        bcl[idx].b1 = idx - 0.4
        bcl[idx].b2 = idx + 0.5
    bclp = biquadCoefficientListParameter(dtype)(bcl)
    assert bclp.size == sz
    for idx in range(sz):
        assert bclp[idx].a1 == bcl[idx].a1
        assert bclp[idx].a2 == bcl[idx].a2
        assert bclp[idx].b0 == bcl[idx].b0
        assert bclp[idx].b1 == bcl[idx].b1
        assert bclp[idx].b2 == bcl[idx].b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_list_parameter_init_from_config(dtype):
    sz = 6
    cfg = pml.VectorParameterConfig(sz)
    bclp = biquadCoefficientListParameter(dtype)(cfg)
    assert bclp.size == sz
    default = biquadCoefficient(dtype)()
    for idx in range(sz):
        bcp = bclp[idx]
        assert bcp.a1 == default.a1
        assert bcp.a2 == default.a2
        assert bcp.b0 == default.b0
        assert bcp.b1 == default.b1
        assert bcp.b2 == default.b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_list_parameter_from_wrong_config_fail(dtype):
    cfg = pml.EmptyParameterConfig()
    with pytest.raises(ValueError):
        bcmp = biquadCoefficientListParameter(dtype)(cfg)

#%% ========================================================================

@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_matrix_parameter_default_init(dtype):
    bcmp = biquadCoefficientMatrixParameter(dtype)()
    assert bcmp.numberOfFilters == 0
    assert bcmp.numberOfSections == 0

@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_matrix_parameter_size_init(dtype):
    rows = 3
    cols = 5
    bcmp = biquadCoefficientMatrixParameter(dtype)(rows, cols)
    assert bcmp.numberOfFilters == rows
    assert bcmp.numberOfSections == cols
    default = biquadCoefficient(dtype)()
    for rowIdx in range(rows):
        for colIdx in range(cols):
            bcp = bcmp[rowIdx, colIdx]
            assert bcp.a1 == default.a1
            assert bcp.a2 == default.a2
            assert bcp.b0 == default.b0
            assert bcp.b1 == default.b1
            assert bcp.b2 == default.b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_matrix_parameter_init_from_nested_list(dtype):
    rows = 2
    cols = 4
    bcnl = []
    for rowIdx in range(rows):
        row = []
        for colIdx in range(cols):
            bc = biquadCoefficient(dtype)(10 * rowIdx + colIdx + 0.1, 10 * rowIdx + colIdx - 0.2,
                                          10 * rowIdx + colIdx + 0.3, 10 * rowIdx + colIdx - 0.4,
                                          10 * rowIdx + colIdx + 0.5)
            row.append(bc)
        bcnl.append(row)
    bcmp = biquadCoefficientMatrixParameter(dtype)(bcnl)
    assert bcmp.numberOfFilters == rows
    assert bcmp.numberOfSections == cols
    for rowIdx in range(rows):
        for colIdx in range(cols):
            bcp = bcmp[rowIdx, colIdx]
            assert bcp.a1 == bcnl[rowIdx][colIdx].a1
            assert bcp.a2 == bcnl[rowIdx][colIdx].a2
            assert bcp.b0 == bcnl[rowIdx][colIdx].b0
            assert bcp.b1 == bcnl[rowIdx][colIdx].b1
            assert bcp.b2 == bcnl[rowIdx][colIdx].b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_matrix_parameter_init_from_nested_list_fail_differing_lengths(dtype):
    rows = 5
    cols = 3
    bcnl = []
    for rowIdx in range(rows):
        row = []
        for colIdx in range(cols):
            bc = biquadCoefficient(dtype)(10 * rowIdx + colIdx + 0.1, 10 * rowIdx + colIdx - 0.2,
                                          10 * rowIdx + colIdx + 0.3, 10 * rowIdx + colIdx - 0.4,
                                          10 * rowIdx + colIdx + 0.5)
            row.append(bc)
        bcnl.append(row)
    # Add one value to the last row -> inner lists have unequal lengths
    bcnl[-1].append(biquadCoefficient(dtype)())
    with pytest.raises(ValueError):
        bcmp = biquadCoefficientMatrixParameter(dtype)(bcnl)


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_matrix_parameter_init_from_rbbl(dtype):
    rows = 3
    cols = 5
    bcm = biquadCoefficientMatrix(dtype)(rows, cols)
    assert bcm.numberOfFilters == rows
    assert bcm.numberOfSections == cols
    for rowIdx in range(rows):
        for colIdx in range(cols):
            bcm[rowIdx, colIdx].a1 = 12 * rowIdx + colIdx + 0.1
            bcm[rowIdx, colIdx].a2 = 12 * rowIdx + colIdx - 0.2
            bcm[rowIdx, colIdx].b0 = 12 * rowIdx + colIdx + 0.3
            bcm[rowIdx, colIdx].b1 = 12 * rowIdx + colIdx - 0.4
            bcm[rowIdx, colIdx].b2 = 12 * rowIdx + colIdx + 0.5
    bcmp = biquadCoefficientMatrixParameter(dtype)(bcm)
    assert bcmp.numberOfFilters == rows
    assert bcmp.numberOfSections == cols
    for rowIdx in range(rows):
        for colIdx in range(cols):
            assert bcm[rowIdx, colIdx].a1 == bcm[rowIdx, colIdx].a1
            assert bcm[rowIdx, colIdx].a2 == bcm[rowIdx, colIdx].a2
            assert bcm[rowIdx, colIdx].b0 == bcm[rowIdx, colIdx].b0
            assert bcm[rowIdx, colIdx].b1 == bcm[rowIdx, colIdx].b1
            assert bcm[rowIdx, colIdx].b2 == bcm[rowIdx, colIdx].b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_matrix_parameter_init_from_config(dtype):
    rows = 6
    cols = 2
    cfg = pml.MatrixParameterConfig(rows, cols)
    bcmp = biquadCoefficientMatrixParameter(dtype)(cfg)
    assert bcmp.numberOfFilters == rows
    assert bcmp.numberOfSections == cols
    default = biquadCoefficient(dtype)()
    for rowIdx in range(rows):
        for colIdx in range(cols):
            bcp = bcmp[rowIdx, colIdx]
            assert bcp.a1 == default.a1
            assert bcp.a2 == default.a2
            assert bcp.b0 == default.b0
            assert bcp.b1 == default.b1
            assert bcp.b2 == default.b2


@pytest.mark.parametrize("dtype", dtypes)
def test_biquad_coefficient_matrix_parameter_from_wrong_config_fail(dtype):
    cfg = pml.VectorParameterConfig(3)
    with pytest.raises(ValueError):
        bcmp = biquadCoefficientMatrixParameter(dtype)(cfg)

# Allow running the tests as a script (as opposed to through pytest)
if __name__ == "__main__":
    for dtype in dtypes:
        test_biquad_coefficient_parameter_default_init(dtype)
        test_biquad_coefficient_parameter_list_init(dtype)
        test_biquad_coefficient_parameter_list_init_wrong_len_fail(dtype)
        test_biquad_coefficient_parameter_scalar_init(dtype)
        test_biquad_coefficient_parameter_scalar_init_kwargs(dtype)
        test_biquad_coefficient_parameter_init_from_rbbl(dtype)
        test_biquad_coefficient_parameter_init_from_config(dtype)
        test_biquad_coefficient_parameter_from_wrong_config_fail(dtype)

        test_biquad_coefficient_list_parameter_default_init(dtype)
        test_biquad_coefficient_list_parameter_size_init(dtype)
        test_biquad_coefficient_list_parameter_init_from_list(dtype)
        test_biquad_coefficient_list_parameter_init_from_rbbl(dtype)
        test_biquad_coefficient_list_parameter_init_from_config(dtype)
        test_biquad_coefficient_list_parameter_from_wrong_config_fail(dtype)

        test_biquad_coefficient_matrix_parameter_default_init(dtype)
        test_biquad_coefficient_matrix_parameter_size_init(dtype)
        test_biquad_coefficient_matrix_parameter_init_from_nested_list(dtype)
        test_biquad_coefficient_matrix_parameter_init_from_nested_list_fail_differing_lengths(dtype)
        test_biquad_coefficient_matrix_parameter_init_from_rbbl(dtype)
        test_biquad_coefficient_matrix_parameter_init_from_config(dtype)
        test_biquad_coefficient_matrix_parameter_from_wrong_config_fail(dtype)
import numpy as np
import pytest
from .irs import fit_delays_below


@pytest.mark.parametrize("smooth", [False, True])
@pytest.mark.parametrize("below", [False, True])
@pytest.mark.parametrize("add_noise", [False, True])
@pytest.mark.parametrize("seed", range(3))
def test_fit_delays_below(smooth, below, add_noise, seed):
    np.random.seed(seed)

    azimuths = np.arange(0, 360, 2)
    x = azimuths / 360

    degree = 7
    poly = np.random.uniform(-1, 1, degree)
    n = np.arange(degree)

    # adjust poly to make y(o) == y(1), and optionally make it smooth at the
    # wrap point

    if smooth:
        # from the power rule at 0 and 1, the derivative at 0 and 1 is
        # equal if:
        #   poly[1] == sum(n[1:] * poly[1:])
        # so adjust poly[-1] to make sum(n[2:] * poly[2:]) equal zero
        poly[-1] = -np.sum(n[2:-1] * poly[2:-1]) / n[-1]

    # make the ends match up; adjusting poly[1] does not disturb the smoothing, above
    # y0: poly[0]
    # y1: sum(poly)
    poly[1] = -np.sum(poly[2:])

    # check that the above worked properly
    if smooth:
        d0 = np.sum(n[1:] * poly[1:] * 0 ** (n[1:] - 1))
        d1 = np.sum(n[1:] * poly[1:] * 1 ** (n[1:] - 1))
        assert abs(d1 - d0) < 1e-6
    y0 = poly[0]
    y1 = np.sum(poly)
    assert abs(y1 - y0) < 1e-6

    y = np.sum(poly[:, np.newaxis] * x[np.newaxis] ** n[:, np.newaxis], axis=0)

    # wrap around so point isn't in middle
    wrap_point = len(x) // 2
    y = np.concatenate((y[wrap_point:], y[:wrap_point]))

    scale = np.max(y) - np.min(y)

    y_noise = y.copy()

    if add_noise:
        if below:
            n_noise = 10
            sample = np.random.choice(np.arange(len(y)), n_noise)
            y_noise[sample] += np.random.uniform(0, 0.1, n_noise)
        else:
            y_noise += np.random.normal(0, 0.01 * scale, len(y))

    fitted_y = fit_delays_below(y_noise, azimuths, below=below, smooth=smooth)

    atol = 1e-5 * scale
    rtol = 1e-5
    if add_noise:
        atol = 0.02 * scale

    if False and not np.allclose(fitted_y, y, atol=atol, rtol=rtol):
        import matplotlib.pyplot as plt

        plt.plot(azimuths, y, label="clean")
        if add_noise:
            plt.plot(azimuths, y_noise, label="noise")
        plt.plot(azimuths, fitted_y, label="fitted")
        plt.plot(azimuths, fitted_y - y, label="diff")
        plt.legend()
        plt.show()

    np.testing.assert_allclose(fitted_y, y, atol=atol, rtol=rtol)

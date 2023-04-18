from scipy import signal
import numpy as np
import random
import simanneal


class AlignCCs(simanneal.Annealer):
    """Align IRs to maximise the cross-correlation between all pairs.

    Parameters:
        cc_mat (ndarray of shape (n, n, m)): matrix of cross-correlations. Zero
            delay between ir[i] and ir[j] is at cc_mat[i, j, m//2].
        peak_choose_lambda (float): parameter controlling the choice of peaks;
            smaller makes it likely that smaller peaks are chosen.
        p_perturb (float): probability of modifying a delay rather than
            aligning to a peak
    """

    def __init__(
        self,
        cc_mat,
        peak_choose_lambda=1.0,
        p_perturb=0.1,
        diff_penalty=None,
        print=True,
        log=False,
    ):
        self.cc_mat = cc_mat
        self.peak_choose_lambda = peak_choose_lambda
        self.p_perturb = p_perturb
        self.diff_penalty = diff_penalty

        self.n = len(self.cc_mat)
        assert cc_mat.shape[-1] % 2 == 1
        self.zero_delay = cc_mat.shape[-1] // 2
        self.sorted_delays = np.argsort(cc_mat, axis=-1)[..., ::-1]

        self.peaks = [
            [sorted(signal.find_peaks(cc)[0], key=lambda p: -cc[p]) for cc in cc_row]
            for cc_row in cc_mat
        ]

        # pre-computed for energy
        self.i, self.j = np.mgrid[: self.n, : self.n]

        # don't call Annealer.__init__ to avoid setting the signal handler
        self.state = np.zeros(self.n, dtype=int)
        self.copy_strategy = "method"

        self.print = print
        if log:
            self.log = []
        else:
            self.log = None

    def init_delays(self, delays):
        self.state = np.round(delays).astype(int)

    def move(self):
        a = random.randrange(len(self.state))

        if np.random.random() < self.p_perturb:
            self.state[a] += 1 if np.random.random() < 0.5 else -1
        else:
            while True:
                b = random.randrange(len(self.state))
                if b == a:
                    continue

                peaks = self.peaks[a][b]
                if not peaks:
                    continue
                i = min(
                    int(random.expovariate(self.peak_choose_lambda)), len(peaks) - 1
                )

                delay_a = self.state[a]
                self.state[b] = delay_a + (peaks[i] - self.zero_delay)
                break

    def energy(self):
        idx = (self.state[self.j] - self.state[self.i]) + self.zero_delay
        idx = np.clip(idx, 0, self.cc_mat.shape[-1] - 1, out=idx)

        ccs = np.take_along_axis(self.cc_mat, idx[..., np.newaxis], axis=-1)[..., 0]

        if self.diff_penalty is not None:
            diffs = np.abs(self.state[:, np.newaxis] - self.state[np.newaxis, :])
            total_diff_penalty = np.sum(diffs * self.diff_penalty)
        else:
            total_diff_penalty = 0.0

        return total_diff_penalty - np.sum(ccs)

    def residual(self, state=None, cc_mat=None):
        if cc_mat is None:
            cc_mat = self.cc_mat
        if state is None:
            state = self.state

        idx = (state[self.j] - state[self.i]) + self.zero_delay
        idx = np.clip(idx, 0, cc_mat.shape[-1] - 1, out=idx)

        ccs = np.take_along_axis(cc_mat, idx[..., np.newaxis], axis=-1)[..., 0]
        return np.max(cc_mat, axis=-1) - ccs

    def update(self, step, T, E, acceptance, improvement):
        if self.log is not None:
            self.log.append(
                dict(
                    step=step,
                    T=T,
                    E=E,
                    acceptance=acceptance,
                    improvement=improvement,
                )
            )

        if self.print:
            self.default_update(step, T, E, acceptance, improvement)


def fit_delays_below(est_delays, azimuths, below=True, smooth=False, degree=6):
    """Fit a polynomial in azimuth to a set of delays.

    The start point (in azimuth) of the polynomial is optimised (line search)
    to find the best fit. The start and end points are constrained to have the
    same value, so while there will be a corner at this point, there will not
    be a step.

    Parameters:
        est_delays (array of n): Array of delays to fit to.
        azimuths (array of n): Azimuth for each delay.
        below (bool): Should the fitted curve all be below the points, or
            should it be allowed to be above and below.
        smooth (bool): should the fitted curve be smooth at the wrap point
            (first derivatives equal), or is a corner allowed?
        degree (int): degree of polynomial to fit
    Returns:
        array of n: fitted delays
    """
    import cvxpy as cp
    from ear.core.geom import relative_angle

    # notes:
    # - this works by fitting a polynomial in angles relative to the start
    #   point, to deal with the wrapping
    # - angles and delays are scaled down to keep the solver happy
    # - to make the problem DPP (and therefore quicker to re-solve) the
    #   parameter is the relative azimuth of each point, taken to each power in
    #   the polynomial. in theory cvxpy could constant-propagate this, but it
    #   doesn't. It might make more sense for the parameter to shifted
    #   estimated delays, instead

    # scale delays to +-0.5
    offset = (np.max(est_delays) + np.min(est_delays)) / 2.0
    scale = np.max(est_delays) - np.min(est_delays)
    est_delays_scale = (est_delays - offset) / scale

    rel_az_p = cp.Parameter((len(est_delays_scale), degree + 1))
    c = cp.Variable(degree + 1)

    # for a single value
    def poly(x):
        return cp.sum(cp.multiply(x ** np.arange(degree + 1), c))

    def poly_derivative(x):
        n = np.arange(1, degree + 1)
        return cp.sum(cp.multiply(n * x ** (n - 1), c[1:]))

    y = cp.sum(cp.multiply(rel_az_p, c[np.newaxis, :]), axis=1)
    objective = cp.Minimize(cp.sum_squares(est_delays_scale - y))

    constraints = [poly(0) == poly(1.0)]
    if below:
        constraints.append(y <= est_delays_scale)
    if smooth:
        constraints.append(poly_derivative(0) == poly_derivative(1.0))

    prob = cp.Problem(objective, constraints)

    results = []
    for centre_az in azimuths:
        rel_az = (
            np.array([relative_angle(centre_az, az) for az in azimuths]) - centre_az
        ) / 360.0
        rel_az_p.value = rel_az[:, np.newaxis] ** np.arange(degree + 1)

        prob.solve(solver=cp.ECOS)

        assert prob.status == "optimal"
        results.append((prob.value, (y.value * scale) + offset))

    return min(results, key=lambda x: x[0])[1]


def norm_cc_mat(cc_mat):
    """Take a matrix of raw cross-correlations and return a normalised one."""
    # computes: ncc[i,j] = cc[i,j] / (max(cc[i, i]) * max(cc[j,j]))
    max_cc = np.sqrt(np.max(np.diagonal(cc_mat), axis=0))
    return (
        cc_mat
        * (1.0 / (max_cc[:, np.newaxis] * max_cc[np.newaxis, :]))[..., np.newaxis]
    )


def get_cc_mat(irs, max_delay=None):
    """Get a matrix of cross-correlations.

    Parameters:
        irs (array of shape n, k): array of n k-length impulse responses
        max_delay (int or None): maximum correlation delay before and after the
            zero-delay sample. If None, return all delays with any overlap.
    Returns:
        array of (n, n, (2*max_delay + 1))
    """
    zero_delay = irs.shape[-1] - 1
    if max_delay is None:
        max_delay = irs.shape[-1] - 1
    delays = np.arange(-max_delay, max_delay + 1)

    return np.array(
        [
            [signal.correlate(ir_a, ir_b, "full")[zero_delay + delays] for ir_a in irs]
            for ir_b in irs
        ]
    )


def estimate_delays(irs, axis=-1, rel_thresh=0.7):
    """Simple delay estimation, finds the location of the first peak larger
    than rel_thresh times the maximum amplitude.

    Parameters:
        irs (ndarray): impulse responses to measure
        axis (int): time axis in irs

    Returns:
        delays (ndarray): integer delays, same shape as irs but with axis
            removed
    """

    def estimate_delay(ir):
        height_thresh = rel_thresh * np.max(np.abs(ir))
        peaks, peak_info = signal.find_peaks(np.abs(ir), height=height_thresh)
        return peaks[0]

    return np.apply_along_axis(estimate_delay, axis, irs)


def est_delays_integral(irs_osa, osa):
    """Estimate delays of impulse responses based on thresholding a scaled
    integral of their energy over time.
    """
    # find the end of the initial part of each IR based on a low threshold and
    # a fixed delay so that we ignore any reflections
    from_thresh = estimate_delays(irs_osa, rel_thresh=0.1)
    win_end = from_thresh + 145 * osa  # about 1m at 48kHz

    # integrate the energy of each IR over time, and normalise so that it's 1
    # at the end of the window
    # TODO: should normalise to zero before the start, too?
    energy_sum = np.cumsum(irs_osa[:, : np.max(win_end) + 1] ** 2, axis=1)
    energy_sum *= 1 / np.take_along_axis(energy_sum, win_end[:, np.newaxis], 1)

    thresh = 0.01
    return np.argmax(energy_sum > thresh, axis=1)


def align_ccs_smooth(
    irs_osa,
    azimuths,
    osa=4,
    sched=None,
    return_annealer=False,
    estimate_delays=est_delays_integral,
    smooth=False,
):
    """Estimate smooth delays for a set of impulse responses for one ear and
    source, sampled over view azimuth.

    This glues together various parts to align the leading edge of the IRs with
    maximum cross-correlation. The leading edge is used as this is the only
    part of the IR which is reliably smooth with azimuth.

    The process is:

    - Estimate the delays simply, to find the approximate leading edge of each
      IR. This estimate tends to have only positive noise (estimate_delays or
      est_delays_integral).
    - Fit a polynomial below these delays to get a smooth leading edge estimate
      (fit_delays_below).
    - Window out a few samples after the leading edge.
    - Use AlignCCs to align the cross-correlations of the windowed IRs, with a
      weighting favouring alignment of close-by IRs.
    - Use fit_delays_below with below=False to smooth any last wrinkles from
      AlignCCs.

    Parameters:
        sched (Optional[dict]): schedule for AlignCCs
        return_annealer (bool): just return the annealer before running it.
            either this or sched must be set
        smooth (bool): should the fitted curve be smooth at the wrap point
            (first derivatives equal), or is a corner allowed? In general, this
            should be set for elevation > 0

    Returns:
        tuple containing the final delays, and a dict of extra intermediate
        values for debugging.
    """
    assert return_annealer or (sched is not None)

    est_delays = estimate_delays(irs_osa, osa)
    smooth_est_delays = fit_delays_below(est_delays, azimuths, smooth=smooth)

    # one-sided raised cosine windows; 1 before start, 0 after end
    def one_window(t, start, end):
        return 0.5 + 0.5 * np.cos(np.interp(t, [start, end], [0, np.pi]))

    def window(t, start, end):
        return np.array([one_window(t, s, e) for s, e in zip(start, end)])

    # window out a short time after the estimated onsets
    # TODO: make this configurable
    win_start = osa * 2.5
    win_end = osa * 5

    # cut out the relevant part to speed up the CC
    # TODO: crop each IR individually to reduce size further
    crop_start = max(0, int(np.min(smooth_est_delays) - osa * 10))
    crop_end = int(np.max(smooth_est_delays) + win_end) + 1

    win = window(
        np.arange(crop_start, crop_end),
        smooth_est_delays + win_start,
        smooth_est_delays + win_end,
    )
    irs_win_short = win * irs_osa[:, crop_start:crop_end]

    cc_mat = norm_cc_mat(get_cc_mat(irs_win_short))

    distances = np.abs(azimuths[:, np.newaxis] - azimuths[np.newaxis, :])
    weights = 1 / (distances + 20)

    acc = AlignCCs(cc_mat * weights[..., np.newaxis], log=True, print=False)
    acc.init_delays(smooth_est_delays)
    if return_annealer:
        return acc

    acc.set_schedule(sched)
    acc.anneal()

    # shift so that the pre- and post-annelaled delays approximately match up
    anneal_delays = acc.best_state - (
        np.min(acc.best_state) - int(round(np.min(smooth_est_delays)))
    )

    fitted = fit_delays_below(
        anneal_delays.astype(float), azimuths, below=False, smooth=smooth
    )
    return (
        fitted,
        dict(
            est_delays=est_delays,
            smooth_est_delays=smooth_est_delays,
            anneal_delays=anneal_delays,
            log=acc.log,
        ),
    )


def apply_delays(signals, delays, axis=-1, osa=1, out_len=None):
    """Apply delays to a set of signals.

    Parameters:
        signals (ndarray): input signals
        delays (ndarray): integer delay to apply to each signal, must have one
            less dimension than signals
        axis (int): axis in signals which represents time and should be delayed
        osa (int): when extending signals, extend by a multiple of osa
        out_len (int): overrides the output length in the time axis; this makes
            using more than one call to apply_delays much simpler

    Returns:
        delayed signals, longer along axis than signals
    """
    assert (
        len(signals.shape) == len(delays.shape) + 1
    ), "signals must have one more dimension than delays"

    # move the time axis to the end
    signals = np.moveaxis(signals, axis, -1)
    delays = delays[..., np.newaxis]
    nsamples = signals.shape[-1]

    signals, delays = np.broadcast_arrays(signals, delays)

    # multiple of osa
    extra_samples = ((np.max(delays) + osa - 1) // osa) * osa

    # make the output array, longer along the time axis
    out_shape = list(signals.shape)
    if out_len is None:
        out_shape[-1] += extra_samples
    else:
        out_shape[-1] = out_len
    out = np.zeros(out_shape)

    for delay_idx in np.ndindex(*delays.shape[:-1]):
        delay = delays[delay_idx][0]
        assert delay >= 0, "delays must be +ve"

        out[delay_idx][delay : delay + nsamples] = signals[delay_idx][:]

    return np.moveaxis(out, -1, axis)

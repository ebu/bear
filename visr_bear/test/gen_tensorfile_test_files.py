import bear.tensorfile
import numpy as np


if __name__ == "__main__":
    idxes = np.mgrid[:2, :4, :8]

    multi_dim = (idxes[0] << 5) + (idxes[1] << 3) + idxes[2]

    data = dict(
        multi_dim=dict(
            float32=multi_dim.astype(np.float32),
            float64=multi_dim.astype(np.float64),
            uint64=multi_dim.astype(np.uint64),
            uint32=multi_dim.astype(np.uint32),
            uint16=multi_dim.astype(np.uint16),
            uint8=multi_dim.astype(np.uint8),
            int64=multi_dim.astype(np.int64),
            int32=multi_dim.astype(np.int32),
            int16=multi_dim.astype(np.int16),
            int8=multi_dim.astype(np.int8),
        ),
    )

    for byte_order in "le", "be":
        for alignment in 1, 32:
            fname = f"test/files/tensorfile_{byte_order}_{alignment}.tenf"
            with open(fname, "wb") as f:
                bear.tensorfile.write(
                    f,
                    data,
                    byte_order=dict(le="<", be=">")[byte_order],
                    alignment=alignment,
                )

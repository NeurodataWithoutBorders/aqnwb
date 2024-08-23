from datetime import datetime
from uuid import uuid4

import numpy as np
from dateutil.tz import tzlocal

from pynwb import NWBHDF5IO, NWBFile
from pynwb.ecephys import LFP, ElectricalSeries

nwbfile = NWBFile(
    session_description="my first synthetic recording",
    identifier=str(uuid4()),
    session_start_time=datetime.now(tzlocal()),
    experimenter=[
        "Baggins, Bilbo",
    ],
    lab="Bag End Laboratory",
    institution="University of Middle Earth at the Shire",
    experiment_description="I went on an adventure to reclaim vast treasures.",
    session_id="LONELYMTN001",
)

device = nwbfile.create_device(
    name="array", description="the best array", manufacturer="Probe Company 9000"
)

nwbfile.add_electrode_column(name="label", description="label of electrode")

nshanks = 4
nchannels_per_shank = 3
electrode_counter = 0

for ishank in range(nshanks):
    # create an electrode group for this shank
    electrode_group = nwbfile.create_electrode_group(
        name="shank{}".format(ishank),
        description="electrode group for shank {}".format(ishank),
        device=device,
        location="brain area",
    )
    # add electrodes to the electrode table
    for ielec in range(nchannels_per_shank):
        nwbfile.add_electrode(
            group=electrode_group,
            label="shank{}elec{}".format(ishank, ielec),
            location="brain area",
        )
        electrode_counter += 1


all_table_region = nwbfile.create_electrode_table_region(
    region=list(range(electrode_counter)),  # reference row indices 0 to N-1
    description="all electrodes",
)


raw_data = np.random.randn(50, 12)
raw_electrical_series = ElectricalSeries(
    name="ElectricalSeries",
    data=raw_data,
    electrodes=all_table_region,
    timestamps=[0.0, 0.1, 0.2, 0.3, 0.4],  # in Hz
)

nwbfile.add_acquisition(raw_electrical_series)

with NWBHDF5IO("ecephys_tutorial.nwb", "w") as io:
    io.write(nwbfile)
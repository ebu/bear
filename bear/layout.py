import ear.core.bs2051
from ear.core.layout import PolarPosition, Channel, Layout
from attrs import evolve


def get_layout(name):
    if name == "9+10+3_extra_rear":
        layout = ear.core.bs2051.get_layout("9+10+3")
        extra_channels = [
            ear.core.layout.Channel(
                name="B+135",
                polar_position=(135.0, -30.0),
                el_range=[-30.0, -15.0],
            ),
            ear.core.layout.Channel(
                name="B-135",
                polar_position=(-135.0, -30.0),
                el_range=[-30.0, -15.0],
            ),
        ]
        return evolve(layout, name=name, channels=layout.channels + extra_channels)
    else:
        return ear.core.bs2051.get_layout(name)


def pos_to_json(pos):
    return dict(azimuth=pos.azimuth, elevation=pos.elevation, distance=pos.distance)


def layout_to_json(layout):
    channels = [
        dict(
            name=channel.name,
            polar_position=pos_to_json(channel.polar_position),
            polar_nominal_position=pos_to_json(channel.polar_nominal_position),
            az_range=list(channel.az_range),
            el_range=list(channel.el_range),
            is_lfe=channel.is_lfe,
        )
        for channel in layout.channels
    ]
    return dict(name=layout.name, channels=channels)


def pos_from_json(pos):
    return PolarPosition(
        azimuth=pos["azimuth"], elevation=pos["elevation"], distance=pos["distance"]
    )


def channel_from_json(channel):
    return Channel(
        name=channel["name"],
        polar_position=pos_from_json(channel["polar_position"]),
        polar_nominal_position=pos_from_json(channel["polar_nominal_position"]),
        az_range=tuple(channel["az_range"]),
        el_range=tuple(channel["el_range"]),
        is_lfe=channel["is_lfe"],
    )


def layout_from_json(layout):
    return Layout(
        channels=[channel_from_json(channel) for channel in layout["channels"]],
        name=layout["name"],
    )


def test_layout_json():
    layout = get_layout("9+10+3")
    layout2 = layout_from_json(layout_to_json(layout))

    assert layout2 == layout

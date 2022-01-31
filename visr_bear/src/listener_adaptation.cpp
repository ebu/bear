#include "listener_adaptation.hpp"

#include <ear/exceptions.hpp>

#include "geom.hpp"

namespace bear {

namespace {
  struct SetPositionVisitor : public boost::static_visitor<void> {
    SetPositionVisitor(const Eigen::Vector3d &position) : position(position) {}

    void operator()(ear::PolarPosition &pos) const
    {
      pos = {azimuth(position), elevation(position), distance(position)};
    }

    void operator()(ear::CartesianPosition &pos) const { pos = {position.x(), position.y(), position.z()}; }

    void operator()(ear::PolarSpeakerPosition &pos) const
    {
      pos = {azimuth(position), elevation(position), distance(position)};
    }

    void operator()(ear::CartesianSpeakerPosition &pos) const
    {
      pos = {position.x(), position.y(), position.z()};
    }

    Eigen::Vector3d position;
  };

  struct CleanPositionVisitor : public boost::static_visitor<void> {
    void operator()(ear::PolarSpeakerPosition &pos) const
    {
      pos.azimuthMin = boost::none;
      pos.azimuthMax = boost::none;
      pos.elevationMin = boost::none;
      pos.elevationMax = boost::none;
      pos.distanceMin = boost::none;
      pos.distanceMax = boost::none;

      if (pos.screenEdgeLock.horizontal) throw ear::not_implemented("DirectSpeakers screenEdgeLock");
      if (pos.screenEdgeLock.vertical) throw ear::not_implemented("DirectSpeakers screenEdgeLock");
    }
    void operator()(ear::CartesianSpeakerPosition &pos) const
    {
      pos.XMin = boost::none;
      pos.XMax = boost::none;
      pos.YMin = boost::none;
      pos.YMax = boost::none;
      pos.ZMin = boost::none;
      pos.ZMax = boost::none;

      if (pos.screenEdgeLock.horizontal) throw ear::not_implemented("DirectSpeakers screenEdgeLock");
      if (pos.screenEdgeLock.vertical) throw ear::not_implemented("DirectSpeakers screenEdgeLock");
    }
  };

}  // namespace

void adapt_otm(const ObjectsInput &block_in, ObjectsInput &block_out, const ListenerImpl &listener)
{
  block_out = block_in;

  boost::optional<double> &absoluteDistance = block_out.audioPackFormat_data.absoluteDistance;

  Eigen::Vector3d position = boost::apply_visitor([](auto pos) { return toCartesianVector3d(pos); },
                                                  block_in.type_metadata.position);

  // BS.2076-2 says that:
  //
  //   If absoluteDistance is negative or undefined, distance based binaural
  //   rendering is not intended.
  //
  // so treat negative as undefined, and don't do any compensation for the
  // listener position if so
  if (absoluteDistance && *absoluteDistance < 0.0) absoluteDistance = boost::none;

  if (absoluteDistance) {
    position *= *absoluteDistance;

    // subtract listener position, to get vector from listener to object
    position -= listener.position;
  }

  double listener_distance = position.norm();

  // undo listener orientation
  position = listener.orientation * position;

  if (absoluteDistance) {
    // the renderer will apply it's own absoluteDistance scaling if it supports it, so the position should be
    // left un-scaled
    position /= *absoluteDistance;
  }

  boost::apply_visitor(SetPositionVisitor(position), block_out.type_metadata.position);

  if (block_in.distance_behaviour) {
    double dist_gain = block_in.distance_behaviour->get_gain(listener_distance, absoluteDistance);
    block_out.type_metadata.gain *= dist_gain;
  }

  // clear out channel locking / screen-related bits
  block_out.type_metadata.channelLock.flag = false;
  block_out.type_metadata.zoneExclusion.zones.clear();
  if (block_out.type_metadata.screenRef) throw ear::not_implemented("Objects screenRef");
}

void adapt_dstm(const DirectSpeakersInput &block_in,
                DirectSpeakersInput &block_out,
                const ListenerImpl &listener)
{
  block_out = block_in;

  Eigen::Vector3d real_position = boost::apply_visitor([](auto pos) { return toCartesianVector3d(pos); },
                                                       block_in.type_metadata.position);

  Eigen::Vector3d position_rel_listener = listener.orientation * real_position;

  boost::apply_visitor(SetPositionVisitor(position_rel_listener), block_out.type_metadata.position);

  // clear out channel locking / screen-related bits
  boost::apply_visitor(CleanPositionVisitor(), block_out.type_metadata.position);
  block_out.type_metadata.speakerLabels.clear();
  block_out.type_metadata.audioPackFormatID = boost::none;
}
}  // namespace bear

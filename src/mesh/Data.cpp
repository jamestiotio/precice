#include "Data.hpp"
#include <algorithm>
#include <utility>

#include "SharedPointer.hpp"
#include "precice/types.hpp"
#include "utils/EigenHelperFunctions.hpp"
#include "utils/assertion.hpp"

namespace precice::mesh {

Data::Data()
    : _name(""),
      _id(-1),
      _dimensions(0),
      _spatialDimensions(-1)
{
  PRECICE_ASSERT(false);
}

Data::Data(
    std::string name,
    DataID      id,
    int         dimensions,
    int         spatialDimensions)
    : _sample(time::Sample{}),
      _name(std::move(name)),
      _id(id),
      _dimensions(dimensions),
      _spatialDimensions(spatialDimensions)
{
  PRECICE_ASSERT(dimensions > 0, dimensions);
}

Eigen::VectorXd &Data::values()
{
  return _sample.values;
}

const Eigen::VectorXd &Data::values() const
{
  return _sample.values;
}

Eigen::MatrixXd &Data::gradients()
{
  return _sample.gradients;
}

const Eigen::MatrixXd &Data::gradients() const
{
  return _sample.gradients;
}

time::Sample &Data::sample()
{
  return _sample;
}

const time::Sample &Data::sample() const
{
  return _sample;
}

time::Storage &Data::timeStepsStorage()
{
  return _timeStepsStorage;
}

void Data::setSampleAtTime(double time, time::Sample sample)
{
  _sample = sample; // @todo at some point we should not need this anymore, when mapping, acceleration ... directly work on _timeStepsStorage
  _timeStepsStorage.setSampleAtTime(time, sample);
}

const std::string &Data::getName() const
{
  return _name;
}

DataID Data::getID() const
{
  return _id;
}

void Data::toZero()
{
  _sample.values.setZero();
  if (_hasGradient) {
    _sample.gradients.setZero();
  }
}

bool Data::hasGradient() const
{
  return _hasGradient;
}

void Data::requireDataGradient()
{
  _hasGradient = true;
};

int Data::getDimensions() const
{
  return _dimensions;
}

void Data::allocateValues(int expectedCount)
{
  using SizeType = std::remove_cv<decltype(expectedCount)>::type;
  // Allocate data values
  const SizeType expectedSize = expectedCount * _dimensions;
  const auto     actualSize   = static_cast<SizeType>(_sample.values.size());
  // Shrink Buffer
  if (expectedSize < actualSize) {
    _sample.values.resize(expectedSize);
  }
  // Enlarge Buffer
  if (expectedSize > actualSize) {
    const auto leftToAllocate = expectedSize - actualSize;
    utils::append(_sample.values, Eigen::VectorXd(Eigen::VectorXd::Zero(leftToAllocate)));
  }
  PRECICE_DEBUG("Data {} now has {} values", _name, _sample.values.size());

  // Allocate gradient data values
  if (_hasGradient) {
    const SizeType spaceDimensions = _spatialDimensions;

    const SizeType expectedColumnSize = expectedCount * _dimensions;
    const auto     actualColumnSize   = static_cast<SizeType>(_sample.gradients.cols());

    // Shrink Buffer
    if (expectedColumnSize < actualColumnSize) {
      _sample.gradients.resize(spaceDimensions, expectedColumnSize);
    }

    // Enlarge Buffer
    if (expectedColumnSize > actualColumnSize) {
      const auto columnLeftToAllocate = expectedColumnSize - actualColumnSize;
      utils::append(_sample.gradients, Eigen::MatrixXd(Eigen::MatrixXd::Zero(spaceDimensions, columnLeftToAllocate)));
    }
    PRECICE_DEBUG("Gradient Data {} now has {} x {} values", _name, _sample.gradients.rows(), _sample.gradients.cols());
  }
}

int Data::getSpatialDimensions() const
{
  return _spatialDimensions;
}

} // namespace precice::mesh

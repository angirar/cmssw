#ifndef FASTSIM_BARRELSIMPLIFIEDGEOMETRY_H
#define FASTSIM_BARRELSIMPLIFIEDGEOMETRY_H

#include "FastSimulation/SimplifiedGeometryPropagator/interface/SimplifiedGeometry.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "TH1F.h"

namespace fastsim{

    class BarrelSimplifiedGeometry : public SimplifiedGeometry
    {
    public:
	~BarrelSimplifiedGeometry(){};
	
	BarrelSimplifiedGeometry(double radius) :
	    SimplifiedGeometry(radius) {}
	
	BarrelSimplifiedGeometry(BarrelSimplifiedGeometry &&) = default;
	
	const double getRadius() const { return position_; }
	
	const double getThickness(const math::XYZTLorentzVector & position, const math::XYZTLorentzVector & momentum) const override
	{
	    if(!this->isOnSurface(position))
	    {
		throw cms::Exception("fastsim::BarrelSimplifiedGeometry::getThickness") << "position is not on layer's surface";
	    }
	    ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >::Scalar> > normVec(position.Px(), position.Py(), 0.);
	    double fabsCosTheta = fabs(momentum.Vect().Dot(normVec)) / (momentum.P() * normVec.R());
	    return thicknessHist_->GetBinContent(thicknessHist_->GetXaxis()->FindBin(fabs(position.Z()))) / fabsCosTheta;
	}
	
	const double getMagneticFieldZ(const math::XYZTLorentzVector & position) const override
	{
	    if(!this->isOnSurface(position))
	    {
		throw cms::Exception("fastsim::BarrelSimplifiedGeometry::getMagneticFieldZ") << "position is not on layer's surface";
	    }
	    return magneticFieldHist_->GetBinContent(magneticFieldHist_->GetXaxis()->FindBin(fabs(position.z())));
	}

	bool isForward() const override 
	{ 
	    return false;
	}

	bool isOnSurface(const math::XYZTLorentzVector & position) const override
	{
	    return fabs(position_ - position.Rho()) < epsilonDistanceR_;
	}
    };

}

#endif

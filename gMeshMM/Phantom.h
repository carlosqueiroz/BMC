#pragma once
#ifndef _PHANTOM_H_
#define _PHANTOM_H_
#include "../Tools/Tools.h"

#define PhantomBufferSize 256*256*256

/**** this class depends highly on your specific problem ****/
class Phantom //define the geometry of the phantom
{
	//methods
public:
	Phantom()
	{
		Hist = 0;
		uniform = 0;
		Bs = 0;
		nBatch = 0;
		prescriptionDose = 0;
		treatmentFraction = 1;
		trimDensityThreshold = 0;
		rngSeed = 1234;
	}
	void loadPhantom(ConfigFile *cf);
	void output(const char* fname, int FormatType=0);
	void getBinaryFile(BinaryFile& BF);
	bool previousDose(const char* fname);
	bool lineInPhantom(Particle& p);

	double getMaxDensity() //when calling this function, make sure that the default density has been loaded
	{
		MaxDensity = ph[0];
		uniform = 1;
		int len = getVoxelNum();
		for (int i = 0; i < len; ++i)
		{
			if (ph[i] != ph[0]) uniform = 0;
			if (MaxDensity < ph[i]) MaxDensity = ph[i];
		}
		if (NMAT > 1) uniform = 0;
		return MaxDensity;
	}
	double getHist(){ return Hist; } //this is for logging purpose
	int getVoxelNum(){ return NX*NY*NZ; }
	double addDose(SFloat* d, SFloat* u, int NB, double fH, double aNorm, double threhold=0.5); //return the square of average uncertainty of threshold% dose region
	double peekUncertainty(SFloat* d, SFloat* u, int NB, double threshold);
	void addDose(SFloat* d, double fhist, double aNorm) //just get dose without uncertainty
	{
		SFloat norm = SFloat(1.602e-16*aNorm / (DX*DY*DZ) / fhist);
		int NVoxel = getVoxelNum();
		for (int i = 0; i < NVoxel; ++i)
		{
			//normalize the additional dose and uncertainty
			dose.a(i) = d[i]*(norm / ph.a(i));
		}
		Hist = fhist;
	}
	int getMatNum()
	{
		int len = matid.getInnerLength();
		vector<short> ids(1);
		ids[0] = matid.a(0);
		for (int i = 1; i < len; ++i)
		{
			int is = (int)ids.size();
			bool found = false;
			for (int j = 0; j < is; ++j)
			{
				if (matid.a(i) == ids[j])
				{
					found = true; 
					break;
				}
			}
			if (!found) ids.push_back(matid.a(i));
		}
		NMAT = (int)ids.size();
		return NMAT;
	}
	int seedBegin(int shift) //it returns previous seed and update rngSeed
	{
		int ret = rngSeed;
		rngSeed += shift;
		return ret;
	}

	//must have I/O data
	int NX, NY, NZ; //voxel number
	double DX, DY, DZ; // voxel size, unit cm
	ArrayMgr<SFloat> ph; //relative density matrix; input

	//optional I/O data
	ArrayMgr<short> matid; //material id matrix; default 0
	ArrayMgr<SFloat> dose; //store average dose per history (not per batch); default 0
	ArrayMgr<SFloat> uncertainty; // store standard deviation based on batch; default 0
	int nBatch; // default 0
	double Hist; // default 0
	double Bs; //magnetic field strength; default 0
	double Bx, By, Bz; //unit magnetic field direction; default (0, 0, -1)
	double xo, yo, zo; // position of the cuboid corner in isocenter coordinate system; determine where's the isocenter
	double COPX, COPY, COPZ;//DICOM coordinates info; determine where's the patient coordinate system's origin
	double prescriptionDose; // used in DoseViewer; default maxDose
	int treatmentFraction; // used in DoseViewer; default 1
	double trimDensityThreshold; // default 0
	int rngSeed;

	//auxiliary data, which can be regenerated by the dose file
	int NMAT; // number of materials in this phantom
	double rf; //radius factor= 100/C/B
	int uniform;
	double MaxDensity;
	double LX, LY, LZ; // side length Lx=DX*NX, not need to output
};

#endif
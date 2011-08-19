				/*
* The information in this file is
* Copyright(c) 2010 Trevor R.H. Clarke
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AppVerify.h"
#include "DataRequest.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "Endian.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "NefImporter.h"

#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "TypeConverter.h"
#include "libraw/libraw/libraw.h"



REGISTER_PLUGIN_BASIC(OpticksTutorial, NefImporter);

namespace
{
   template<typename T>
   T getVal(FILE* pFp, Endian& e)
   {
      T tmp;
      fread(&tmp, sizeof(T), 1, pFp);
      e.swapValue(tmp);
      return tmp;
   }
}

NefImporter::NefImporter()
{
   setDescriptorId("{cb8fc80b-4fcd-4c13-8183-61ec61e95cb2}");
   setName("NEF Importer");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright("abc");
   setVersion("0.1");
   setProductionStatus(false);
   setExtensions("NEF Filez (*.nef)");
}

NefImporter::~NefImporter()
{
}

std::vector<ImportDescriptor*> NefImporter::getImportDescriptors(const std::string &filename)
{
   std::vector<ImportDescriptor*> descriptors;

   try
   {
     	
	   
	   
	   ImportDescriptorResource pImportDescriptor(filename, TypeConverter::toString<RasterElement>());
	   VERIFYRV(pImportDescriptor.get() != NULL, descriptors);
	   descriptors.push_back(pImportDescriptor.release());
	   
	   InterleaveFormatType interleave(BSQ);
	   EncodingType encoding(INT4UBYTES);
	   
	   //bool rgb = false;
	   
	   LibRaw iProcessor;
	   const char *fn=filename.c_str();
	   iProcessor.open_file(fn);

	   unsigned long rows = iProcessor.imgdata.sizes.iheight, columns = iProcessor.imgdata.sizes.iwidth,frames=3;
	   
	   RasterDataDescriptor *pDescriptor = RasterUtilities::generateRasterDataDescriptor(
      filename, NULL, rows, columns, frames, interleave, encoding, IN_MEMORY);

	  if(pDescriptor != NULL)
      {
      
         pDescriptor->setDisplayBand(RED, pDescriptor->getBands()[0]);
         pDescriptor->setDisplayBand(GREEN, pDescriptor->getBands()[1]);
         pDescriptor->setDisplayBand(BLUE, pDescriptor->getBands()[2]);
         pDescriptor->setDisplayMode(RGB_MODE);
      }

	  
	  pImportDescriptor->setDataDescriptor(pDescriptor);
	  RasterUtilities::generateAndSetFileDescriptor(pImportDescriptor->getDataDescriptor(), filename, std::string(), LITTLE_ENDIAN_ORDER);
	  descriptors.push_back(pImportDescriptor.release());
	  //return descriptors;

	   
	   /*   
	   Endian e;
      DynamicObject* pMeta = NULL;
      { // scope the geotiff importer
         ImporterResource geotiff("GeoTIFF Importer", filename);
         if (geotiff.get() == NULL)
         {
            return descriptors;
         }
         std::vector<ImportDescriptor*> tiffDescriptors = geotiff->getImportDescriptors();
         if (tiffDescriptors.size() != 1)
         {
            return descriptors;
         }
         e = Endian(tiffDescriptors.front()->getDataDescriptor()->getFileDescriptor()->getEndian());
         pMeta = tiffDescriptors.front()->getDataDescriptor()->getMetadata();
      }
      if (dv_cast<std::string>(pMeta->getAttributeByPath("TIFF/Make")) != "NIKON CORPORATION")
      {
         return descriptors;
      }
	  

      // Reload the file and parse the RAW IFD
      FileResource pNef(filename.c_str(), "rb");
      if (pNef.get() == NULL)
      {
         return descriptors;
      }
      // JpegImageOffset, RawOffset
      std::vector<unsigned int> ifds = dv_cast<std::vector<unsigned int> >(pMeta->getAttributeByPath("TIFF/SubIFD"));
      if (ifds.size() != 2)
      {
         return descriptors;
      }
      fseek(pNef, ifds[1], SEEK_SET);
      
      unsigned int rows = 0;
      unsigned int cols = 0;
      unsigned int bpp = 0;
      // parse the entries
      size_t entryCount = getVal<uint16_t>(pNef, e);
      while (--entryCount >= 0)
      {
         uint16_t tag = getVal<uint16_t>(pNef, e);
         uint16_t type = getVal<uint16_t>(pNef, e);
         uint16_t count = getVal<uint32_t>(pNef, e);
         bool compressed = false;
         switch(tag)
         {
         case 254: // SubfileType == 0 (full resolution)
            if (type != 4 && count != 1 && getVal<uint32_t>(pNef, e) != 0)
            {
               return descriptors;
            }
            break;
         case 256: // ImageWidth
            if (type != 4 && count != 1)
            {
               return descriptors;
            }
            cols = getVal<uint32_t>(pNef, e);
            break;
         case 257: // ImageHight
            if (type != 4 && count != 1)
            {
               return descriptors;
            }
            rows = getVal<uint32_t>(pNef, e);
            break;
         case 258: // BitsPerSample
            if (type != 1 && count != 1)
            {
               return descriptors;
            }
            bpp = getVal<unsigned char>(pNef, e);
            fseek(pNef, 3, SEEK_CUR);
            break;
         case 259: // Compression
            if (type != 3 && count != 1)
            {
               return descriptors;
            }
            {
               uint16_t comp = getVal<uint16_t>(pNef, e);
               fseek(pNef, 2, SEEK_CUR);
               if (comp == 1)
               {
                  compressed = false;
               }
               else if (comp == 34713)
               {
                  compressed = true;
               }
               else
               {
                  return descriptors;
               }
            }
            break;
         default:
            fseek(pNef, 4, SEEK_CUR);
            break;
         }
      }

	  */
   }
   catch (const std::bad_cast&)
   {
      // metadata not present, wrong kind of file
   }
   return descriptors;
}

unsigned char NefImporter::getFileAffinity(const std::string &filename)
{
   try
   {  /*
      ImporterResource geotiff("GeoTIFF Importer", filename);
      if (geotiff.get() == NULL)
      {
         return CAN_NOT_LOAD;
      }
      std::vector<ImportDescriptor*> tiffDescriptors = geotiff->getImportDescriptors();
      if (tiffDescriptors.size() != 1)
      {
         return CAN_NOT_LOAD;
      }
      DynamicObject* pMeta = tiffDescriptors.front()->getDataDescriptor()->getMetadata();
      if (dv_cast<std::string>(pMeta->getAttributeByPath("TIFF/Make")) != "NIKON CORPORATION")
      {
         return CAN_NOT_LOAD;

      }
	  */
	   LibRaw jProcessor;
	   const char *fname=filename.c_str();
       if(jProcessor.open_file(fname)== LIBRAW_SUCCESS)
	   {
      return CAN_LOAD + 100;
	   }
   }
   catch (const std::bad_cast&)
   {
      // metadata not present, wrong kind of file
   }
   return CAN_NOT_LOAD;
}


bool NefImporter::getInputSpecification(PlugInArgList* &pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Importer::ImportElementArg(), "RAW image importer");
   return true;
}



bool NefImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
    
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   //setting up mpRasterElement
   parseInputArgList(pInArgList);
   RasterElement* pRaster = getRasterElement();
   
   VERIFY(pRaster != NULL);
   RasterDataDescriptor *pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());

   VERIFY(pDescriptor != NULL);
   FileDescriptor *pFileDescriptor = pDescriptor->getFileDescriptor();
   VERIFY(pFileDescriptor != NULL);

   //data accessor
   //RED
   DimensionDescriptor firstBand = pDescriptor->getActiveBand(0);
   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   pRequest->setBands(firstBand, firstBand);
   pRequest->setWritable(true);
   DataAccessor firstBandDa = pRaster->getDataAccessor(pRequest.release());

   
   //GREEN
   DimensionDescriptor secondBand = pDescriptor->getActiveBand(1);
   FactoryResource<DataRequest> qRequest;
   qRequest->setInterleaveFormat(BSQ);
   qRequest->setBands(secondBand, secondBand);
   qRequest->setWritable(true);
   DataAccessor secondBandDa = pRaster->getDataAccessor(qRequest.release());

   //BLUE
   DimensionDescriptor thirdBand = pDescriptor->getActiveBand(2);
   FactoryResource<DataRequest> rRequest;
   rRequest->setInterleaveFormat(BSQ);
   rRequest->setBands(thirdBand, thirdBand);
   rRequest->setWritable(true);
   DataAccessor thirdBandDa = pRaster->getDataAccessor(rRequest.release());

   

   std::string filename = pRaster->getFilename();
   Progress *pProgress = getProgress();
   
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(filename);

   LibRaw RawProcessor;
   putenv ((char*)"TZ=UTC"); 

	#define P1  RawProcessor.imgdata.idata
#define S   RawProcessor.imgdata.sizes
#define C   RawProcessor.imgdata.color
#define T   RawProcessor.imgdata.thumbnail
#define P2  RawProcessor.imgdata.other
#define OUT RawProcessor.imgdata.params

   const char *fname=filename.c_str();
    RawProcessor.open_file(fname);

	// Let us unpack the image
        if (RawProcessor.unpack() != LIBRAW_SUCCESS)
   {
      return false;
   }
    
    /*
	unsigned int *r=NULL;
	unsigned int *g=NULL;
	unsigned int *b=NULL;
	unsigned int *h=NULL;
	*/
    unsigned int *zero=0;
	int row=0,col=0,r=0,c=0;
	
		   
		   /*
		   r=(unsigned int*)(&RawProcessor.imgdata.image[i][0]);
           g=(unsigned int*)(&RawProcessor.imgdata.image[i][1]);
           b=(unsigned int*)(&RawProcessor.imgdata.image[i][2]);
           h=(unsigned int*)(&RawProcessor.imgdata.image[i][3]);
		   */
		   //secondBandDa->toPixel(row,col);
		   //thirdBandDa->toPixel(row,col);

		   
		unsigned short *pData=reinterpret_cast<unsigned short*>(pRaster->getRawData());
		if (pData == NULL)
		{
      return NULL;
		}

		memcpy(pData, RawProcessor.imgdata.rawdata.raw_image, sizeof(unsigned short) * RawProcessor.imgdata.sizes.raw_height * RawProcessor.imgdata.sizes.raw_width);


		 /*  
		   if(i%2==0) //RG1
		       {memcpy(firstBandDa->getColumn(),(unsigned int*)RawProcessor.imgdata.image[i][0],sizeof(unsigned int));
				memcpy(thirdBandDa->getColumn(),(unsigned int*)RawProcessor.imgdata.image[i][2],sizeof(unsigned int));
				memcpy(secondBandDa->getColumn(),zero,sizeof(unsigned int));
			    }
		   else{
			   //G2B
			   memcpy(thirdBandDa->getColumn(),(unsigned int*)RawProcessor.imgdata.image[i][3],sizeof(unsigned int));
			   memcpy(secondBandDa->getColumn(),(unsigned int*)RawProcessor.imgdata.image[i][1],sizeof(unsigned int));
			   memcpy(firstBandDa->getColumn(),zero,sizeof(unsigned int));
				}

		*/
		unsigned short *ptr=NULL;

		//band 0 Red
		for(row=0,r=0;row<S.iheight;row++,r++)
		{
			for(col=0,c=0;col<S.iwidth;col++,c++)
			{
			   if(row%2==0)  //RG row
			   {
			    if(col%2==0) //Red pixel
				{
					ptr=reinterpret_cast<unsigned short*>(firstBandDa->getColumn());
				    *ptr=pData[c+(r*S.iwidth)+(0*S.iheight*S.iwidth)];
			     }
			    else
			    {
				   *ptr=0;
				   c--;
			    }
			   }
			   else  //GB row
			   {
				   *ptr=0;
			   }
			    firstBandDa->nextColumn();
			   
			}
			if(row%2!=0)
			   r--;
			firstBandDa->nextRow();
		}
		

		//band 2 Blue
		for(row=0,r=0;row<S.iheight;row++,r++)
		{
			for(col=0,c=0;col<S.iwidth;col++,c++)
			{
			   if(row%2!=0)  //GB row
			   {
			    if(col%2!=0) //Blue pixel
				{
					ptr=reinterpret_cast<unsigned short*>(secondBandDa->getColumn());
				    *ptr=pData[c+(r*S.iwidth)+(2*S.iheight*S.iwidth)];
			     }
			    else
			    {
				   *ptr=0;
				   c--;
			    }
			   }
			   else  //RG row
			   {
				   *ptr=0;
			   }
			    secondBandDa->nextColumn();
			   
			}
			if(row%2==0)
			   r--;
			secondBandDa->nextRow();
		}


		//band 1 Green
		for(row=0,r=0;row<S.iheight;row++,r++)
		{
			for(col=0,c=0;col<S.iwidth;col++,c++)
			{
			   if(row%2==0)  //RG row
			   {
			    if(col%2!=0) //Green pixel
				{
					ptr=reinterpret_cast<unsigned short*>(thirdBandDa->getColumn());
				    *ptr=pData[c+(r*S.iwidth)+(1*S.iheight*S.iwidth)]; //g1
			     }
			    else
			    {
				   *ptr=0;
				   c--;
			    }
			   }
			   else  //GB row
			   {
				   if(col%2==0) //Green pixel
				{
					ptr=reinterpret_cast<unsigned short*>(thirdBandDa->getColumn());
				    *ptr=pData[c+(r*S.iwidth)+(3*S.iheight*S.iwidth)]; //g2
			     }
			    else
			    {
				   *ptr=0;
				   c--;
			    }
				   
			   }
			    thirdBandDa->nextColumn();
			   
			}
			
			thirdBandDa->nextRow();
		}


			
	if (createView() == NULL)
    {
      return false;
    }
	

	RawProcessor.recycle();
   
   return true;
}




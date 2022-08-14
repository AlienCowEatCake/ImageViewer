
#pragma once

// You can include this file (if you work in C++) but you don't have to. 
// If you call this api from another language (Python, C#), you see integers. 
// This header file tells you what these integers mean. 
// These enum's may grow in the future. More values can be added. 

namespace LercNS
{
  enum ErrCode
  {
    ErrCode_Ok = 0,
    ErrCode_Failed,
    ErrCode_WrongParam,
    ErrCode_BufferTooSmall,
    ErrCode_NaN,
    ErrCode_HasNoData
  };

  enum DataType
  {
    DataType_dt_char = 0,
    DataType_dt_uchar,
    DataType_dt_short,
    DataType_dt_ushort,
    DataType_dt_int,
    DataType_dt_uint,
    DataType_dt_float,
    DataType_dt_double
  };

  enum InfoArrOrder
  {
    InfoArrOrder_version = 0,    // codec version
    InfoArrOrder_dataType,
    InfoArrOrder_nDim,    // = nDepth (we renamed nDim to nDepth but don't want to break anything)
    InfoArrOrder_nCols,
    InfoArrOrder_nRows,
    InfoArrOrder_nBands,
    InfoArrOrder_nValidPixels,  // for 1st band
    InfoArrOrder_blobSize,
    InfoArrOrder_nMasks,  // 0 - all valid, 1 - same mask for all bands, nBands - masks can differ between bands
    InfoArrOrder_nDepth,  // = nDim (we renamed nDim to nDepth but don't want to break anything)
    InfoArrOrder_nUsesNoDataValue,  // 0 - no noData value used, nBands - noData value used in 1 or more bands (only possible for nDepth > 1)
    InfoArrOrder__last
  };

  enum DataRangeArrOrder
  {
    DataRangeArrOrder_zMin = 0,
    DataRangeArrOrder_zMax,
    DataRangeArrOrder_maxZErrUsed,
    DataRangeArrOrder__last
  };

}    // namespace


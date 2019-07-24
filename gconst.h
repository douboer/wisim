#ifndef GConst_H
#define GConst_H

class GConst
{
public:

    enum CanvasSize {
        callSize = 10
    };

    /**************************************************************************************/
    /**** Z Values                                                                     ****/
    /**************************************************************************************/
    // Note that polygon regions use two Z-values, z for polygon and z+1 for edges.
    enum ZValue {
                           clutterZ = 0,
                            heightZ = 0,
                             layerZ = 0,
                        backgroundZ = 0,
                   mapLayerPolygonZ = 5,
                    systemBoundaryZ = 10,
                      selectRegionZ = 20,
                        connectionZ = 25,
                              cellZ = 30,
                          roadTestZ = 40,
                          cellTextZ = 45
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** RTTI Values                                                                  ****/
    /**************************************************************************************/
    enum RTTIValue {
                           cellRTTI = 2000,
         polygonBoundarySegmentRTTI = 2003,
                  polygonRegionRTTI = 2004,

                 systemBoundaryRTTI = 2005,
                 callConnectionRTTI = 2008,
                 rulerLineClassRTTI = 2009,
                   rulerPtClassRTTI = 2010,

               backgroundPixmapRTTI = 2011,
                     mapClutterRTTI = 2012,
                      mapHeightRTTI = 2013,
                       mapLayerRTTI = 2014,
                  mapBackgroundRTTI = 2015,
                    coverageTopRTTI = 2016,
                       coverageRTTI = 2017,
                   roadTestDataRTTI = 2018,

                       cellTextRTTI = 2019,

                 visibilityItemRTTI = 2020,
            visibilityCheckItemRTTI = 2021,
                        trafficRTTI = 2022,
                         subnetRTTI = 2023,
                        antennaRTTI = 2024,

               clutterPropModelRTTI = 2025
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Mouse modes                                                                  ****/
    /**************************************************************************************/
    enum MouseMode {
                         selectMode,
                    toggleNoiseMode,
                           zoomMode,
                        addCellMode,
                addRoadTestDataMode,
                       measPathMode,
                         scrollMode,
                    drawPolygonMode,
                       moveCellMode,
                       copyCellMode,
                      trackCellMode,
                    trackSubnetMode,
           trackMapLayerPolygonMode,
              trackRoadTestDataMode,
               trackSignalLevelMode,
                   trackClutterMode,
          trackClutterPropModelMode,
                    ModClutterCoeff,
                     rulerStartMode,
                      rulerMeasMode,
// xxxxxxxxxxxxx    selectGroupMode,
                     powerMeterMode
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Application actions                                                          ****/
    /**************************************************************************************/
    enum ApplicationAction {
                      geometryClose,
                    applicationQuit
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Mouse actions                                                                ****/
    /**************************************************************************************/
    enum MouseAction {
                         mousePress = 1,
                          mouseMove = 2,
                       mouseRelease = 3,
                   mouseDoubleClick = 4
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Visibility actions                                                           ****/
    /**************************************************************************************/
    enum VisibilityAction {
                         visHide    = 0,
                         visShow    = 1,
                         visToggle  = 2
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** File / Report Types                                                          ****/
    /**************************************************************************************/
    enum FileType {
                       geometryFile,
                       mapLayerFile,
                     mapClutterFile,
               clutterPropModelFile,
                      mapHeightFile,
                   roadTestDataFile,
               coverageAnalysisFile,
                   cchRSSITableFile,
               mapBackgroundTabFile,
              imageRegistrationFile,
                            BMPFile,
                            JPGFile,
                       handoverFile,

                   statisticsReport,
                     coverageReport,
                       coveragePlot,
                       subnetReport,
             propagationErrorReport,
             propagationParamReport,
                     settingsReport
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Unit Types                                                                   ****/
    /**************************************************************************************/
    enum UnitType {
                           wattUnit,
                      milliWattUnit,
                            dBWUnit,
                            dBmUnit,
                           dBuVUnit
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** Window Types                                                                 ****/
    /**************************************************************************************/
    enum WindowType {
                      commandWindow,
                   visibilityWindow,
                         infoWindow
    };
    /**************************************************************************************/

    /**************************************************************************************/
    /**** PropModMgrDia states                                                         ****/
    /**************************************************************************************/
    enum PropModMgrDiaState {
                          editState,
                          viewState
    };
    /**************************************************************************************/
};

#endif

0、该版本SDK支持网口、光口、无线版本平板探测器开发；
1、将HBI_DLL文件夹拷贝到“DemoDll\DemoDll”或“Demolib\Demolib”或“HB_SDK_DEMO2013\HB_SDK_DEMO2008”目录下；
2、“HB_SDK_DEMO2013_单板”工程依赖Opencv341库，直接可以拷贝《opencv341_解压到D盘.7z》到D盘直接解压，解压目录为D:\opencv341\opencv；
3、部分debug模式下依赖库opencv需要其他库，例如msvcp100d.dll或msvcr100d.dll或msvcp120d.dll或msvcr120d.dll可以在“HB_SDK_DEMO2013_单板\output”目录下找到32位或64位库文件，注意32位和64位，见《依赖库截图.png》，可联系研发人员获取；
4、生成目录为：工程目录\output\Win32和工程目录\output\x64，具体参考HB_SDK_DEMO2013工程；
4、HB_SDK_DEMO2008工程是基于MFC Dialog开发，包含显示、生成模板等，功能最全，DemoDll和Demolib基于控制台consol开发，包含了基础功能，其中DemoDll为动态加载Demo，Demolib为静态为静态加载Demo，具体可以参考工程文档说明，例如《昊博SDK开发DEMO工程说明Ver4.0.1.6.pdf》；
5、需要集成“双板”解决方案的用户，可以联系售后或研发人员索要开发DEMO工程源码；
6、另外需要C# SDK Demo请联系售后或研发人员；

0. This version of the SDK supports the development of flat panel detectors in network, optical and wireless versions;

1. HBI_ Copy the DLL folder to the "DemoDll DemoDll" or "Demolib Demolib" or "HB_SDK_DEMO2013 HB_SDK_DEMO2008" directory;

2. The "HB_SDK_DEMO2013_Single Board" project relies on the Opencv341 library. You can directly copy the "opencv341 Unzipping to Disk D. 7z" to Disk D and unzip it directly. The unzipping directory is D: opencv341 opencv;

3. In some debug modes, the dependent library opencv requires other libraries, such as msvcp100d.dll or msvcr100d.dll or msvcp120d.dll or msvcr120d.dll. You can find 32-bit or 64-bit library files in the directory "HB_SDK_DEMO2013_ Board output". Pay attention to 32-bit and 64-bit. See the screenshot of the dependent library. png, which can be obtained by contacting the R&D personnel;

4. The generated directories are: project directory output Win32 and project directory output x64. Refer to HB for details_ SDK_ DEMO2013 project;

4、HB_ SDK_ DEMO2008 project is developed based on MFC Dialog, including display, template generation, etc., with the most comprehensive functions. DemoDll and Demolib are developed based on console console, including basic functions. DemoDll is dynamically loaded Demo, Demolib is statically loaded Demo. For details, please refer to the project document description, such as "Haobo SDK Development DEMO Project Description Ver4.0.1.6.pdf";

5. Users who need to integrate the "dual board" solution can contact after-sales or R&D personnel to request the development of DEMO project source code;

6. In addition, if you need C # SDK Demo, please contact after-sales or R&D personnel;
xcopy /y/s		.\Engine\Public\*.*					.\Reference\Headers\
xcopy /y		.\Engine\Bin\*.lib					.\Reference\Librarys\
xcopy /y		.\Engine\ThirdPartyLib\*.lib		.\Reference\Librarys\
xcopy /y		.\Engine\ThirdPartyLib\*.dll		.\MainApp\Bin\
xcopy /y		.\Engine\Bin\*.dll					.\MainApp\Bin\
xcopy /y		.\Engine\Bin\ShaderFiles\*.*		.\MainApp\Bin\ShaderFiles\
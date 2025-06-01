xcopy /y/s		.\Engine\Public\*.*					.\Reference\Headers\
xcopy /y		.\Engine\Bin\*.lib					.\Reference\Librarys\
xcopy /y		.\Engine\ThirdPartyLib\*.lib		.\Reference\Librarys\
xcopy /y		.\Engine\ThirdPartyLib\*.dll		.\MainApp\Bin\
xcopy /y		.\Engine\Bin\*.dll					.\MainApp\Bin\
xcopy /y/s		.\PhysicsEngine\PhysX-4.1\*.*		.\Reference\Headers\PhysX-4.1
xcopy /y		.\PhysicsEngine\bin\*.lib			.\Reference\Librarys\
#include "TMDockTabHelper.h"



#include "Framework/Docking/TabManager.h"

#include "Widgets/Docking/SDockTab.h"



namespace

{

    int32 GTMDockTabSerial = 0;

}



TSharedPtr<SDockTab> TMDockTab::OpenDockTab(

    const TCHAR* Prefix,

    const FText& Title,

    const TSharedRef<SWidget>& Content,

    FSimpleDelegate OnClosed)

{

    const FName TabId(*FString::Printf(TEXT("TraceMotive.%s.%d"), Prefix, ++GTMDockTabSerial));



    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(

        TabId,

        FOnSpawnTab::CreateLambda([TabId, Title, Content, OnClosed](const FSpawnTabArgs&)

        {

            return SNew(SDockTab)

                .TabRole(ETabRole::NomadTab)

                .Label(Title)

                .OnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda([TabId, OnClosed](TSharedRef<SDockTab>) mutable

                {

                    OnClosed.ExecuteIfBound();

                    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);

                }))

                [

                    Content

                ];

        }))

        .SetDisplayName(Title)

        .SetMenuType(ETabSpawnerMenuType::Hidden);



    return FGlobalTabmanager::Get()->TryInvokeTab(TabId);

}


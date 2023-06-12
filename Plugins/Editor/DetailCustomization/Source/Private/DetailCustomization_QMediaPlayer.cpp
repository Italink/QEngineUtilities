#include "DetailCustomization_QMediaPlayer.h"
#include "DetailView/QPropertyHandle.h"
#include "DetailView/QDetailViewManager.h"
#include "Widgets/QMediaPlayerEditor.h"


void DetailCustomization_QMediaPlayer::CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
	QMediaPlayer* player = (QMediaPlayer*)Context.ObjectPtr;
	QMediaPlayerEditor* editor = new QMediaPlayerEditor;
	editor->SetupPlayer(player);
	Builder->AddRowByWholeContent(editor);
}

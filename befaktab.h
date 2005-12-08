
#ifndef _BEFAKTAB_H
#define _BEFAKTAB_H

#include <sqlite.h>

class BHandler;
class BMessage;
class BTab;
class BTabView;
class BView;

class beFakTab {
	public:
		beFakTab(BTabView *tv, sqlite *db, BHandler *hr);
		virtual ~beFakTab();

		virtual void MessageReceived(BMessage *Message);
		virtual bool CommitCurdata(bool haveCancelButton = true);
		virtual void DoCommitCurdata(void) { };	// INSERT/UPDATE
		virtual bool validateTab(void) { return true; };

		virtual void curdataFromTab(void) { };
		virtual void curdataToTab(void) { };
		virtual void updateTab(void) { };

		const char *execSQL(const char *input);
		const char *validateDecimal(const char *input);
		const char *validateDate(const char *input);

		BHandler *handler;
		BView *view;
		BTab *tab;

		sqlite *dbData;
		char *dbErrMsg;

	protected:
		bool dirty;
};

#define MSG_NAMECHANGE	'BFNC'
#define MSG_REQTOWARUP	'BFRT'
#define MSG_REQFIRMAUP	'BFRF'

#endif

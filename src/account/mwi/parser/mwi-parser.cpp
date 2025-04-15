/*
 * Copyright (c) 2010-2024 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mwi-parser.h"
#include "account/mwi/message-waiting-indication-summary.h"
#include "linphone/utils/utils.h"
#include "logger/logger.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

namespace {
string MwiGrammar("mwi_grammar.belr");
}

namespace Mwi {

class MessageSummaryNode;
class MessageAccountNode;
class SummaryLineNode;

class MessageStatusNode : public Node {
public:
	friend MessageSummaryNode;

	void setMsgStatus(const string &status) {
		mMsgStatus = (status == "yes");
	}

private:
	bool mMsgStatus;
};

// -------------------------------------------------------------------------

class MessageAccountNode : public Node {
public:
	friend MessageSummaryNode;

	void setAccountUri(const string &uri) {
		mAddress = Address::create(uri);
	}

private:
	shared_ptr<Address> mAddress;
};

// -------------------------------------------------------------------------

class SummaryLineNode : public Node {
public:
	friend MessageSummaryNode;

	void setMessageContext(const string &context) {
		if (Utils::iequals(context, "voice-message")) {
			mMessageContext = LinphoneMessageWaitingIndicationVoice;
		} else if (Utils::iequals(context, "fax-message")) {
			mMessageContext = LinphoneMessageWaitingIndicationFax;
		} else if (Utils::iequals(context, "pager-message")) {
			mMessageContext = LinphoneMessageWaitingIndicationPager;
		} else if (Utils::iequals(context, "multimedia-message")) {
			mMessageContext = LinphoneMessageWaitingIndicationMultimedia;
		} else if (Utils::iequals(context, "text-message")) {
			mMessageContext = LinphoneMessageWaitingIndicationText;
		} else if (Utils::iequals(context, "none")) {
			mMessageContext = LinphoneMessageWaitingIndicationNone;
		}
	}

	void setNewMsgs(const string &newMsgs) {
		mNewMsgs = static_cast<uint32_t>(std::stoul(newMsgs));
	}

	void setOldMsgs(const string &oldMsgs) {
		mOldMsgs = static_cast<uint32_t>(std::stoul(oldMsgs));
	}

	void setNewUrgentMsgs(const string &newUrgentMsgs) {
		mNewUrgentMsgs = static_cast<uint32_t>(std::stoul(newUrgentMsgs));
	}

	void setOldUrgentMsgs(const string &oldUrgentMsgs) {
		mOldUrgentMsgs = static_cast<uint32_t>(std::stoul(oldUrgentMsgs));
	}

private:
	LinphoneMessageWaitingIndicationContextClass mMessageContext = LinphoneMessageWaitingIndicationNone;
	uint32_t mNewMsgs;
	uint32_t mOldMsgs;
	uint32_t mNewUrgentMsgs;
	uint32_t mOldUrgentMsgs;
};

// -------------------------------------------------------------------------

class MessageSummaryNode : public Node {
public:
	void setMessageStatus(const shared_ptr<MessageStatusNode> &messageStatus) {
		mMessageStatus = messageStatus;
	}

	void setAccount(const shared_ptr<MessageAccountNode> &account) {
		mAccount = account;
	}

	void addSummaryLine(const shared_ptr<SummaryLineNode> &summaryLine) {
		mSummaryLines.push_back(summaryLine);
	}

	// Warning: Call this function one time!
	shared_ptr<MessageWaitingIndication> createMessageWaitingIndication() const {
		shared_ptr<MessageWaitingIndication> mwi = MessageWaitingIndication::create();
		if (mMessageStatus) {
			mwi->setMessageWaiting(mMessageStatus->mMsgStatus);
		}
		if (mAccount) {
			if (mAccount->mAddress && mAccount->mAddress->isValid()) {
				mwi->setAccountAddress(mAccount->mAddress);
			} else {
				lWarning() << "Cannot set account address to mwi because the address is null or invalid.";
				return nullptr;
			}
		}
		for (auto line : mSummaryLines) {
			mwi->addSummary(MessageWaitingIndicationSummary::create(
			    line->mMessageContext, line->mOldMsgs, line->mNewMsgs, line->mOldUrgentMsgs, line->mNewUrgentMsgs));
		}
		return mwi;
	}

private:
	shared_ptr<MessageStatusNode> mMessageStatus;
	shared_ptr<MessageAccountNode> mAccount;
	list<shared_ptr<SummaryLineNode>> mSummaryLines;
};

} // namespace Mwi

// -------------------------------------------------------------------------

Mwi::Parser::Parser() {
	shared_ptr<belr::Grammar> grammar = belr::GrammarLoader::get().load(MwiGrammar);
	if (!grammar) lFatal() << "Unable to load MWI grammar.";
	mParser = make_shared<belr::Parser<shared_ptr<Node>>>(grammar);

	mParser->setHandler("message-summary", belr::make_fn(make_shared<MessageSummaryNode>))
	    ->setCollector("msg-status-line", belr::make_sfn(&MessageSummaryNode::setMessageStatus))
	    ->setCollector("msg-account", belr::make_sfn(&MessageSummaryNode::setAccount))
	    ->setCollector("msg-summary-line", belr::make_sfn(&MessageSummaryNode::addSummaryLine));

	mParser->setHandler("msg-status-line", belr::make_fn(make_shared<MessageStatusNode>))
	    ->setCollector("msg-status", belr::make_sfn(&MessageStatusNode::setMsgStatus));

	mParser->setHandler("msg-account", belr::make_fn(make_shared<MessageAccountNode>))
	    ->setCollector("Account-URI", belr::make_sfn(&MessageAccountNode::setAccountUri));

	mParser->setHandler("msg-summary-line", belr::make_fn(make_shared<SummaryLineNode>))
	    ->setCollector("message-context-class", belr::make_sfn(&SummaryLineNode::setMessageContext))
	    ->setCollector("newmsgs", belr::make_sfn(&SummaryLineNode::setNewMsgs))
	    ->setCollector("oldmsgs", belr::make_sfn(&SummaryLineNode::setOldMsgs))
	    ->setCollector("new-urgentmsgs", belr::make_sfn(&SummaryLineNode::setNewUrgentMsgs))
	    ->setCollector("old-urgentmsgs", belr::make_sfn(&SummaryLineNode::setOldUrgentMsgs));
}

// -----------------------------------------------------------------------------

shared_ptr<Mwi::MessageWaitingIndication> Mwi::Parser::parseMessageSummary(const string &input) {
	size_t parsedSize;
	shared_ptr<Node> node = mParser->parseInput("message-summary", input, &parsedSize);
	if (!node) {
		lWarning() << "Unable to parse message-summary.";
		return nullptr;
	}

	shared_ptr<MessageSummaryNode> messageSummaryNode = dynamic_pointer_cast<MessageSummaryNode>(node);
	if (!messageSummaryNode) {
		lWarning() << "Unable to cast belr result to message summary node.";
		return nullptr;
	}

	shared_ptr<MessageWaitingIndication> mwi = messageSummaryNode->createMessageWaitingIndication();
	return mwi;
}

LINPHONE_END_NAMESPACE

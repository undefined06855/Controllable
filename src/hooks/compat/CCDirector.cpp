#include "CCDirector.hpp"

void HookedCCDirector::setNotificationNode(cocos2d::CCNode* node) {
    if (!node) {
        CCDirector::setNotificationNode(nullptr);
        return;
    }

    if (node->getUserObject("is-special-and-important"_spr)) {
        CCDirector::setNotificationNode(node);
        return;
    }

    geode::log::warn(R"(
Attempted to set notification node by another mod! If you wish to have access
over the notification node, please fill out a full legal document containing
your reasoning behind needing said notification node, as well as any reason you
cannot hook any other draw or visit functions, and any reason you cannot just
place your node above other nodes in the scene, and use Geode's SceneManager's
persistent node feature. Additionally, this form must also include a monetary
bribe in the form of raw geode stone of a minimum worth of 420 Great British
Pounds (GBP). It must be signed, in blood, by yourself as well as every member
of the British Monarchy, past, present and future, as well as every Lead
Developer of the Geode Lead Developer team.
I do hope that this will not be too much of a hassle, and I apologise for this
extra light security measure.

Alternatively, ping me on Discord, @undefined06855
)");
}

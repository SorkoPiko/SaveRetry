#include <Geode/modify/AccountLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GJAccountManager.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class $modify(SRAccountLayer, AccountLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* m_cancelButton;
        CCMenu* m_cancelMenu;
        CCLabelBMFont* m_backupLabel;
        int m_attempts = 1;
        bool m_cancelled = false;
    };

    void customSetup() {
        AccountLayer::customSetup();

        m_fields->m_cancelMenu = CCMenu::create();
        m_fields->m_cancelMenu->setPosition({30.f, 295.f});

        m_fields->m_backupLabel = CCLabelBMFont::create(
            fmt::format("Attempt {}", m_fields->m_attempts).c_str(),
            "goldFont.fnt"
        );

        m_fields->m_backupLabel->setScale(0.8f);
        m_fields->m_backupLabel->setPosition({
            m_linkedAccountTitle->getPositionX(),
            m_linkedAccountTitle->getPositionY() + 30.f
        });
        m_fields->m_backupLabel->setAnchorPoint({0.5f, 0.5f});

        const auto sprite = CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png");
        m_fields->m_cancelButton = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(SRAccountLayer::cancelBackup)
        );
        m_fields->m_cancelButton->setID("cancel-button"_spr);

        m_fields->m_cancelMenu->addChild(m_fields->m_cancelButton);
        customHideLoadingUI();
        m_mainLayer->addChild(m_fields->m_cancelMenu, 106);
        m_mainLayer->addChild(m_fields->m_backupLabel, 106);
    }

    void cancelBackup(CCObject* sender) {
        const auto notif = Notification::create(
            "Backup Cancelled - Finishing Last Attempt",
            NotificationIcon::Info
        );
        notif->show();
        m_fields->m_cancelButton->setVisible(false);
        m_fields->m_cancelMenu->setEnabled(false);
        m_fields->m_cancelled = true;
    }

    void customShowLoadingUI() {
        m_fields->m_cancelMenu->setEnabled(true);
        m_fields->m_backupLabel->setString(fmt::format("Attempt {}", m_fields->m_attempts).c_str());
        m_fields->m_backupLabel->setVisible(true);
        m_fields->m_cancelButton->setVisible(true);
    }

    void customHideLoadingUI() {
        m_fields->m_attempts = 1;
        m_fields->m_cancelMenu->setEnabled(false);
        m_fields->m_backupLabel->setVisible(false);
        m_fields->m_cancelButton->setVisible(false);
    }

    void incrementAttempt() {
        m_fields->m_attempts++;
        m_fields->m_backupLabel->setString(fmt::format("Attempt {}", m_fields->m_attempts).c_str());
    }

    void showAttempts() {
        std::string label = fmt::format("Backup successful\n({} attempts)", m_fields->m_attempts);
        if (m_fields->m_attempts == 1) {
            label = "Backup successful\n(1 attempt)";
        }
        m_textArea->setString(label);
        m_textArea->colorAllCharactersTo({0, 255, 0});
    }

    void backupAccountFailed(const BackupAccountError p0, const int p1) {
        log::info("{} {}", static_cast<int>(p0), p1);

        if (static_cast<int>(p0) == -1 && !m_fields->m_cancelled) {
            incrementAttempt();
            const auto gjam = GJAccountManager::sharedState();
            const float profile = gjam->m_gameManagerSize * 0.00000095367;
            const float levels = gjam->m_localLevelsSize * 0.00000095367;
            if (profile + levels > 32.0) {
                customHideLoadingUI();
                m_fields->m_attempts = 1;
                AccountLayer::backupAccountFailed(p0, p1);
                return;
            }
            this->doBackup();
            return;
        }
        customHideLoadingUI();

        AccountLayer::backupAccountFailed(p0, p1);
    }

    void backupAccountFinished() {
        AccountLayer::backupAccountFinished();
        showAttempts();
        customHideLoadingUI();
    }

    void syncAccountFailed(const BackupAccountError p0, const int p1) {
        log::info("{} {}", static_cast<int>(p0), p1);

        if (static_cast<int>(p0) == -1 && !m_fields->m_cancelled) {
            incrementAttempt();
            this->doSync();
            return;
        }
        customHideLoadingUI();

        AccountLayer::syncAccountFailed(p0, p1);
    }

    void syncAccountFinished() {
        AccountLayer::syncAccountFinished();
        showAttempts();
        customHideLoadingUI();
    }

    void FLAlert_Clicked(FLAlertLayer *p0, bool p1) {
        AccountLayer::FLAlert_Clicked(p0, p1);

        if (p1 == true) {
            customShowLoadingUI();
        }
    }
};
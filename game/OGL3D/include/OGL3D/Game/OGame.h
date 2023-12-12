
Class OWindow;
class OGame
{
public:
    OGame();
    ~OGame();

    void run();
    void quit();
protected:
    bool m_isRunning = true;
    OWindow* m_display = nullptr;
};


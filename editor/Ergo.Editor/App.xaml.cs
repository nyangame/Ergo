using Ergo.Editor.Views;

namespace Ergo.Editor;

public partial class App : Application
{
    public App()
    {
        InitializeComponent();
    }

    protected override Window CreateWindow(IActivationState? activationState)
    {
        return new Window(new MainEditorPage())
        {
            Title = "Ergo Editor",
            Width = 1600,
            Height = 900,
        };
    }
}

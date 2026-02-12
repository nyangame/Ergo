using Ergo.Editor.Interop;
using Ergo.Editor.ViewModels;

namespace Ergo.Editor.Views;

public partial class ComponentView : ContentView
{
    private readonly ComponentViewModel _viewModel = new();

    public ComponentView()
    {
        BindingContext = _viewModel;
        InitializeComponent();
    }

    /// <summary>
    /// Update the inspector to show the selected object's components.
    /// Called from the main editor page when selection changes.
    /// </summary>
    public void SetSelectedObject(ErgoGameObjectHandle handle)
    {
        _viewModel.SelectObject(handle);
    }
}

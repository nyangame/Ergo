using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Ergo.Editor.Interop;

namespace Ergo.Editor.ViewModels;

/// <summary>
/// Represents a single component attached to a game object.
/// </summary>
public class ComponentEntry : INotifyPropertyChanged
{
    public string Name { get; set; } = "";
    public string TypeName { get; set; } = "";
    public ObservableCollection<PropertyEntry> Properties { get; } = new();

    public event PropertyChangedEventHandler? PropertyChanged;
}

/// <summary>
/// Represents a single editable property on a component.
/// </summary>
public class PropertyEntry : INotifyPropertyChanged
{
    private string _valueText = "";

    public string Name { get; set; } = "";
    public ErgoPropertyType Type { get; set; }

    public string ValueText
    {
        get => _valueText;
        set
        {
            if (_valueText == value) return;
            _valueText = value;
            OnPropertyChanged();
        }
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

/// <summary>
/// ViewModel for the component inspector panel.
/// Queries the native engine for components on the selected game object
/// and presents them as an editable list.
/// </summary>
public class ComponentViewModel : INotifyPropertyChanged
{
    private ErgoGameObjectHandle _selectedObject;
    private string _objectName = "(No selection)";

    // -----------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------

    public string ObjectName
    {
        get => _objectName;
        private set
        {
            if (_objectName == value) return;
            _objectName = value;
            OnPropertyChanged();
        }
    }

    public ObservableCollection<ComponentEntry> Components { get; } = new();

    public bool HasSelection => _selectedObject.IsValid;

    // Transform (editable directly in the inspector)
    private ErgoTransform3D _transform = ErgoTransform3D.Identity;

    public float PosX { get => _transform.Position.X; set { _transform.Position.X = value; OnPropertyChanged(); ApplyTransform(); } }
    public float PosY { get => _transform.Position.Y; set { _transform.Position.Y = value; OnPropertyChanged(); ApplyTransform(); } }
    public float PosZ { get => _transform.Position.Z; set { _transform.Position.Z = value; OnPropertyChanged(); ApplyTransform(); } }

    public float ScaleX { get => _transform.Scale.X; set { _transform.Scale.X = value; OnPropertyChanged(); ApplyTransform(); } }
    public float ScaleY { get => _transform.Scale.Y; set { _transform.Scale.Y = value; OnPropertyChanged(); ApplyTransform(); } }
    public float ScaleZ { get => _transform.Scale.Z; set { _transform.Scale.Z = value; OnPropertyChanged(); ApplyTransform(); } }

    // -----------------------------------------------------------------
    // Selection
    // -----------------------------------------------------------------

    public void SelectObject(ErgoGameObjectHandle handle)
    {
        _selectedObject = handle;
        OnPropertyChanged(nameof(HasSelection));

        if (!handle.IsValid)
        {
            ObjectName = "(No selection)";
            Components.Clear();
            return;
        }

        ObjectName = EngineInterop.GetObjectName(handle);
        _transform = EngineInterop.GetObjectTransform(handle);
        OnPropertyChanged(nameof(PosX));
        OnPropertyChanged(nameof(PosY));
        OnPropertyChanged(nameof(PosZ));
        OnPropertyChanged(nameof(ScaleX));
        OnPropertyChanged(nameof(ScaleY));
        OnPropertyChanged(nameof(ScaleZ));

        RefreshComponents();
    }

    public void RefreshComponents()
    {
        Components.Clear();
        if (!_selectedObject.IsValid) return;

        uint count = EngineInterop.GetComponentCount(_selectedObject);
        if (count == 0) return;

        var infos = new ErgoComponentInfo[count];
        uint fetched = EngineInterop.GetComponents(_selectedObject, infos, count);

        for (uint i = 0; i < fetched; i++)
        {
            Components.Add(new ComponentEntry
            {
                Name = infos[i].NameStr,
                TypeName = infos[i].TypeNameStr,
            });
        }
    }

    // -----------------------------------------------------------------
    // Internal
    // -----------------------------------------------------------------

    private void ApplyTransform()
    {
        if (!_selectedObject.IsValid) return;
        EngineInterop.SetObjectTransform(_selectedObject, _transform);
    }

    // -----------------------------------------------------------------
    // INotifyPropertyChanged
    // -----------------------------------------------------------------

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

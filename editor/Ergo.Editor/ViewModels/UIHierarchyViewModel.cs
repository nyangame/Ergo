using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Ergo.Editor.Interop;

namespace Ergo.Editor.ViewModels;

/// <summary>
/// ViewModel for the UI editor hierarchy panel.
/// Provides a tree-view of UICanvas and UINode objects with
/// creation, deletion, reparenting, and property editing.
/// </summary>
public class UIHierarchyViewModel : INotifyPropertyChanged
{
    private UIHierarchyEntry? _selectedEntry;

    // -----------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------

    public ObservableCollection<UIHierarchyEntry> HierarchyEntries { get; } = new();

    public UIHierarchyEntry? SelectedEntry
    {
        get => _selectedEntry;
        set
        {
            _selectedEntry = value;
            OnPropertyChanged();
            OnPropertyChanged(nameof(HasSelection));
            OnPropertyChanged(nameof(IsCanvasSelected));
            RefreshSelectedProperties();
        }
    }

    public bool HasSelection => _selectedEntry != null;
    public bool IsCanvasSelected => _selectedEntry?.NodeType == ErgoUINodeType.Canvas;

    // Properties of selected node
    public string SelectedName
    {
        get => _selectedEntry != null
            ? EngineInterop.UIGetNodeName(_selectedEntry.Handle)
            : "";
        set
        {
            if (_selectedEntry == null) return;
            EngineInterop.UISetNodeName(_selectedEntry.Handle, value);
            _selectedEntry.Name = value;
            OnPropertyChanged();
        }
    }

    public ErgoUIScaleMode SelectedScaleMode
    {
        get
        {
            if (_selectedEntry?.NodeType != ErgoUINodeType.Canvas)
                return ErgoUIScaleMode.DotByDot;
            return EngineInterop.UIGetCanvasScaleMode(_selectedEntry.Handle);
        }
        set
        {
            if (_selectedEntry?.NodeType != ErgoUINodeType.Canvas) return;
            EngineInterop.UISetCanvasScaleMode(_selectedEntry.Handle, value);
            OnPropertyChanged();
        }
    }

    public ErgoUIRectTransform SelectedRectTransform
    {
        get => _selectedEntry != null
            ? EngineInterop.UIGetRectTransform(_selectedEntry.Handle)
            : default;
        set
        {
            if (_selectedEntry == null) return;
            EngineInterop.UISetRectTransform(_selectedEntry.Handle, value);
            OnPropertyChanged();
        }
    }

    // -----------------------------------------------------------------
    // Canvas creation
    // -----------------------------------------------------------------

    public ErgoUINodeHandle CreateCanvas(string name = "Canvas")
    {
        var handle = EngineInterop.UICreateCanvas(name);
        RefreshHierarchy();
        return handle;
    }

    public void RemoveCanvas(ErgoUINodeHandle handle)
    {
        EngineInterop.UIRemoveCanvas(handle);
        if (_selectedEntry?.Handle.Id == handle.Id)
            SelectedEntry = null;
        RefreshHierarchy();
    }

    // -----------------------------------------------------------------
    // Node creation
    // -----------------------------------------------------------------

    public ErgoUINodeHandle CreateNode(string name = "Node")
    {
        if (_selectedEntry == null) return default;
        var handle = EngineInterop.UICreateNode(_selectedEntry.Handle, name);
        RefreshHierarchy();
        return handle;
    }

    public ErgoUINodeHandle CreateImageNode(
        string name = "Image", string texturePath = "")
    {
        if (_selectedEntry == null) return default;
        var handle = EngineInterop.UICreateImageNode(
            _selectedEntry.Handle, name, texturePath);
        RefreshHierarchy();
        return handle;
    }

    public void RemoveSelectedNode()
    {
        if (_selectedEntry == null) return;

        if (_selectedEntry.NodeType == ErgoUINodeType.Canvas)
            EngineInterop.UIRemoveCanvas(_selectedEntry.Handle);
        else
            EngineInterop.UIRemoveNode(_selectedEntry.Handle);

        SelectedEntry = null;
        RefreshHierarchy();
    }

    // -----------------------------------------------------------------
    // Hierarchy operations
    // -----------------------------------------------------------------

    public void Reparent(ErgoUINodeHandle node, ErgoUINodeHandle newParent)
    {
        EngineInterop.UIReparent(node, newParent);
        RefreshHierarchy();
    }

    public void SetSiblingIndex(ErgoUINodeHandle node, int index)
    {
        EngineInterop.UISetSiblingIndex(node, index);
        RefreshHierarchy();
    }

    // -----------------------------------------------------------------
    // Canvas properties
    // -----------------------------------------------------------------

    public void SetCanvasScaleMode(
        ErgoUINodeHandle canvas, ErgoUIScaleMode mode)
    {
        EngineInterop.UISetCanvasScaleMode(canvas, mode);
        OnPropertyChanged(nameof(SelectedScaleMode));
    }

    public void SetCanvasReferenceResolution(
        ErgoUINodeHandle canvas, float width, float height)
    {
        EngineInterop.UISetCanvasReferenceResolution(canvas, width, height);
    }

    public void SetCanvasScreenMatchMode(
        ErgoUINodeHandle canvas, ErgoUIScreenMatchMode mode)
    {
        EngineInterop.UISetCanvasScreenMatchMode(canvas, mode);
    }

    // -----------------------------------------------------------------
    // Refresh hierarchy from native side
    // -----------------------------------------------------------------

    public void RefreshHierarchy()
    {
        HierarchyEntries.Clear();

        uint count = EngineInterop.UIGetHierarchyCount();
        if (count == 0) return;

        var infos = new ErgoUINodeInfo[count];
        uint fetched = EngineInterop.UIGetHierarchy(infos, count);

        for (uint i = 0; i < fetched; i++)
        {
            HierarchyEntries.Add(new UIHierarchyEntry
            {
                Handle = infos[i].Handle,
                ParentHandle = infos[i].Parent,
                NodeType = infos[i].NodeType,
                Name = infos[i].NameStr,
                Depth = infos[i].Depth,
                ChildCount = infos[i].ChildCount,
                IsActive = infos[i].Active != 0,
                IsVisible = infos[i].Visible != 0,
            });
        }
    }

    // -----------------------------------------------------------------
    // Private
    // -----------------------------------------------------------------

    private void RefreshSelectedProperties()
    {
        OnPropertyChanged(nameof(SelectedName));
        OnPropertyChanged(nameof(SelectedScaleMode));
        OnPropertyChanged(nameof(SelectedRectTransform));
    }

    // -----------------------------------------------------------------
    // INotifyPropertyChanged
    // -----------------------------------------------------------------

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

/// <summary>
/// A single entry in the flattened UI hierarchy list.
/// </summary>
public class UIHierarchyEntry : INotifyPropertyChanged
{
    private string _name = "";
    private bool _isActive = true;
    private bool _isVisible = true;

    public ErgoUINodeHandle Handle { get; set; }
    public ErgoUINodeHandle ParentHandle { get; set; }
    public ErgoUINodeType NodeType { get; set; }

    public string Name
    {
        get => _name;
        set { _name = value; OnPropertyChanged(); }
    }

    public int Depth { get; set; }
    public int ChildCount { get; set; }

    public bool IsActive
    {
        get => _isActive;
        set
        {
            _isActive = value;
            EngineInterop.UISetNodeActive(Handle, value ? 1 : 0);
            OnPropertyChanged();
        }
    }

    public bool IsVisible
    {
        get => _isVisible;
        set
        {
            _isVisible = value;
            EngineInterop.UISetNodeVisible(Handle, value ? 1 : 0);
            OnPropertyChanged();
        }
    }

    /// <summary>
    /// Display indent string based on tree depth.
    /// </summary>
    public string IndentedName => new string(' ', Depth * 4) + NodeTypeIcon + " " + Name;

    private string NodeTypeIcon => NodeType switch
    {
        ErgoUINodeType.Canvas => "[C]",
        ErgoUINodeType.Image => "[I]",
        _ => "[-]",
    };

    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}

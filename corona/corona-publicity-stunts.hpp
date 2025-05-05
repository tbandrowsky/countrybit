#pragma once


#ifndef CORONA_PUBLICITY_STUNTS
#define CORONA_PUBLICITY_STUNTS

namespace corona
{

    class CoronaDropSource : public IDropSource {
    public:
        // Implement IUnknown methods
        HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override {
            if (riid == IID_IUnknown || riid == IID_IDropSource) {
                *ppv = static_cast<IDropSource*>(this);
                AddRef();
                return S_OK;
            }
            *ppv = nullptr;
            return E_NOINTERFACE;
        }

        ULONG __stdcall AddRef() override {
            return InterlockedIncrement(&m_refCount);
        }

        ULONG __stdcall Release() override {
            ULONG count = InterlockedDecrement(&m_refCount);
            if (count == 0) {
                delete this;
            }
            return count;
        }

        // Implement IDropSource methods
        HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override {
            if (fEscapePressed) return DRAGDROP_S_CANCEL;
            if (!(grfKeyState & MK_LBUTTON)) return DRAGDROP_S_DROP;
            return S_OK;
        }

        HRESULT __stdcall GiveFeedback(DWORD dwEffect) override {
            return DRAGDROP_S_USEDEFAULTCURSORS;
        }

    private:
        LONG m_refCount = 1;
    };

    // Function to start the drag-and-drop operation
    void StartDragDropOperation() {
        CoronaDropSource* pDropSource = new CoronaDropSource();
        IDataObject* pDataObject = nullptr; // CreateMyDataObject(); // Implement this function to create your data object
        DWORD dwEffect;
        DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
        pDropSource->Release();
        pDataObject->Release();
    }

}

#endif

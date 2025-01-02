#pragma once
#include <NsGui/XamlProvider.h>
namespace api {
    class UI_API NoesisXamlProvider : public Noesis::XamlProvider
    {
    public:
        NoesisXamlProvider();
        ~NoesisXamlProvider();

    private:
        /// From XamlProvider
        //@{
        Noesis::Ptr<Noesis::Stream> LoadXaml(const Noesis::Uri& uri) override;
        //@}

    };
}
#pragma once
#include <NsGui/FontProvider.h>
namespace api {
    class UI_API NoesisFontProvider : public Noesis::FontProvider
    {
    public:
        NoesisFontProvider();
        ~NoesisFontProvider();

    private:
        /// From FontProvider
        //@{
        Noesis::FontSource MatchFont(const Noesis::Uri& baseUri, const char* familyName,
            Noesis::FontWeight& weight, Noesis::FontStretch& stretch, Noesis::FontStyle& style) override;
        bool FamilyExists(const Noesis::Uri& baseUri, const char* familyName) override;
        //@}
    };
}
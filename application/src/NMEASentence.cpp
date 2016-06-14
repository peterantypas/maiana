#include "NMEASentence.hpp"
#include "Utils.hpp"

NMEASentence::NMEASentence(const string &raw)
        : mRaw(raw)
{
    parse();
}

const string &NMEASentence::code() const
{
    return mCode;
}

const string &NMEASentence::raw() const
{
    return mRaw;
}

const vector<string> &NMEASentence::fields() const
{
    return mFields;
}

bool NMEASentence::parse()
{
    if ( mRaw.empty() )
        return false;

    if ( mRaw[0] != '$' && mRaw[0] != '!' )
        return false;

    Utils::tokenize(mRaw, ',', mFields);
    mFields[0] = mFields[0].substr(1); // Remove the $ or ! from the first field
    string &lastField = *mFields.rbegin();

    size_t p = lastField.find("*");
    lastField = lastField.substr(0, p);
    mCode = mFields[0];

    return true;
}

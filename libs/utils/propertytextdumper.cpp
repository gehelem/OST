/**
 * @file propertytextdumper.cpp
 * @brief Human-readable text serialization implementation
 * @author OST Development Team
 * @version 1.0
 */

#include "propertytextdumper.h"
#include "elementtextdumper.h"
#include <sstream>
namespace  OST
{

/**
 * @brief Convert common property metadata to text
 * @param pProperty Pointer to property base
 * @return std::string with formatted metadata
 *
 * Formats common metadata as key=value pairs:
 * - Label (display name)
 * - Level1/Level2 (hierarchy)
 * - Permission (0=ReadOnly, 1=WriteOnly, 2=ReadWrite)
 * - State (0=Idle, 1=Ok, 2=Busy, 3=Error)
 *
 * Used for debugging and logging output.
 */
std::string PropertyTextDumper::dumpPropertyCommons(PropertyBase *pProperty)
{
    std::stringstream stream;
    stream << " Label='" << pProperty->label().toStdString() << "'"
           << " Level1='" << pProperty->level1().toStdString() << "'"
           << " Level2='" << pProperty->level2().toStdString() << "'"
           << " Permission=" << pProperty->permission()
           << " State=" << pProperty->state()
           ;
    return stream.str();
}

/**
 * @brief Convert PropertyMulti to human-readable text
 * @param pProperty Pointer to PropertyMulti to convert
 *
 * Main visitor implementation that converts PropertyMulti to formatted text.
 *
 * Output format:
 * - Property type indicator ("Multi Property")
 * - Common metadata from dumpPropertyCommons()
 * - All elements converted via ElementTextDumper
 * - Each element separated by " **** " delimiter
 * - Element format: "key : element_text//"
 * - Terminated with " #####" marker
 *
 * Result stored in mResult member (QString), accessible via getResult().
 *
 * @par Example Output:
 * @code
 * Multi Property : Label='Focus' Level1='Control' Level2='Focus' Permission=2 State=1.
 * values= **** iterations : int=10// **** exposure : float=5.0// #####
 * @endcode
 *
 * Useful for console debugging and log files.
 *
 * @see ElementTextDumper
 */
void PropertyTextDumper::visit(PropertyMulti *pProperty)
{
    std::stringstream stream;
    stream << "Multi Property :" << dumpPropertyCommons(pProperty)
           << ". values=";

    foreach(const QString &key, pProperty->getElts()->keys())
    {
        ElementTextDumper d;

        pProperty->getElt(key)->accept(&d);
        stream << " **** " << key.toStdString() << " : " << d.getResult().toStdString() << "//";
    }



    stream << " #####";
    mResult = QString::fromStdString(stream.str());
}

}

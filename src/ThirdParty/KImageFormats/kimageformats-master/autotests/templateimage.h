/*
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TEMPLATEIMAGE_H
#define TEMPLATEIMAGE_H

#include <QFileInfo>
#include <QImage>

/*!
 * \brief The TemplateImage class
 * Given an image name, it decides the template image to compare it with.
 */
class TemplateImage
{
public:
    enum TestFlag {
        None = 0x0,
        SkipTest = 0x1,
        DisableAutotransform = 0x2
    };
    Q_DECLARE_FLAGS(TestFlags, TestFlag)

    /*!
     * \brief TemplateImage
     * \param fi The image to test.
     */
    TemplateImage(const QFileInfo& fi);

    /*!
     * \brief TemplateImage
     * Default copy constructor.
     */
    TemplateImage(const TemplateImage& other) = default;
    /*!
     * \brief operator =
     * Default copy operator
     */
    TemplateImage& operator=(const TemplateImage& other) = default;

    /*!
     * \brief isTemplate
     * \return True if the image is a template, false otherwise.
     * \sa suffixes
     */
    bool isTemplate() const;

    /*!
     * \brief isLicense
     * \return True if the file suffix is .license
     */
    bool isLicense() const;

    /*!
     * \brief skipSequentialDeviceTest
     * \return tre it the sequential test should be skipped.
     */
    bool skipSequentialDeviceTest() const;

    /*!
     * \brief compareImage
     * \param flags Flags for modifying test behavior (e.g. image format not supported by current Qt version).
     * \return The template image to use for the comparison.
     */
    QFileInfo compareImage(TestFlags &flags, QString& comment) const;

    /*!
     * \brief checkOptionaInfo
     * Verify the optional information (resolution, metadata, etc.) of the
     * image with that in the template if present.
     * \param image The image to check optional information on.
     * \param error The error message when returns false.
     * \return True on success, otherwise false.
     */
    bool checkOptionaInfo(const QImage& image, QString& error) const;

    /*!
     * \brief fuzziness
     * The fuzziness value that ensures the test works correctly. Normally
     * set for lossy codecs and images that require floating point
     * conversions.
     * Floating point conversions may give slightly different results from
     * one architecture to another (Intel, PowerPC, Arm, etc...).
     * \return The default fuzziness value for the image. Zero means no fuzziness.
     */
    quint8 fuzziness() const;

    /*!
     * \brief perceptiveFuzziness
     * The perceptual mode of fuzziness control scales the value according to
     * the alpha value: the lower the alpha value (transparent), the more the
     * fuzziness value increases according to the following formula:
     * - fuzz = fuzz * 255 / alpha
     * \return True if the perceptive mode is active, otherwise false.
     */
    bool perceptiveFuzziness() const;

    /*!
     * \brief suffixes
     * \return The list of suffixes considered templates.
     */
    static QStringList suffixes();

private:
    /*!
     * \brief legacyImage
     * \return The template image calculated from the source image name.
     */
    QFileInfo legacyImage() const;

    /*!
     * \brief jsonImage
     * \param flags Flags for modifying test behavior.
     * \return The template image read from the corresponding JSON.
     */
    QFileInfo jsonImage(TestFlags &flags, QString& comment) const;

private:
    QFileInfo m_fi;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TemplateImage::TestFlags)

#endif // TEMPLATEIMAGE_H

// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Implements the basic image functionality.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include "image.h"
#include "framebuffer.h"
#include "logging.h"
#include "ndspy.h"

START_NAMESPACE( Aqsis )

CqImage::~CqImage()
{
	free(m_data);
	free(m_realData);
}

void CqImage::PrepareImageBuffer()
{
	//boost::mutex::scoped_lock lock(mutex());
	m_data = reinterpret_cast<unsigned char*>(malloc( m_imageWidth * m_imageHeight * numChannels() * sizeof(TqUchar)));
	// Initialise the display to a checkerboard to show alpha
	for (TqUlong i = 0; i < imageHeight(); i ++)
	{
		for (TqUlong j = 0; j < imageWidth(); j++)
		{
			int     t       = 0;
			TqUchar d = 255;

			if ( ( (imageHeight() - 1 - i) & 31 ) < 16 )
				t ^= 1;
			if ( ( j & 31 ) < 16 )
				t ^= 1;
			if ( t )
				d      = 128;
			for(TqUint chan = 0; chan < numChannels(); chan++)
				data()[numChannels() * (i*imageWidth() + j) + chan ] = d;
		}
	}
	// Now prepare the buffer for the natural data.
	// First work out how big each element is by scanning the channels specification.
	m_elementSize = 0;
	for(std::vector<std::pair<std::string, TqInt> >::iterator channel = m_channels.begin(); channel != m_channels.end(); ++channel)
	{
		switch(channel->second)
		{
			case PkDspyFloat32:
			case PkDspyUnsigned32:
			case PkDspySigned32:
				m_elementSize += 4;
				break;
			case PkDspyUnsigned16:
			case PkDspySigned16:
				m_elementSize += 2;
				break;
			case PkDspyUnsigned8:
			case PkDspySigned8:
			default:
				m_elementSize += 1;
				break;
		}
	}
	m_realData = reinterpret_cast<unsigned char*>(malloc( m_imageWidth * m_imageHeight * m_elementSize));
}

void CqImage::setUpdateCallback(boost::function<void(int,int,int,int)> f)
{
	m_updateCallback = f;
}

TiXmlElement* CqImage::serialiseToXML()
{
	TiXmlElement* imageXML = new TiXmlElement("Image");

	TiXmlElement* typeXML = new TiXmlElement("Type");
	TiXmlText* typeText = new TiXmlText("external");
	typeXML->LinkEndChild(typeText);
	imageXML->LinkEndChild(typeXML);

	TiXmlElement* nameXML = new TiXmlElement("Name");
	TiXmlText* nameText = new TiXmlText(name());
	nameXML->LinkEndChild(nameText);
	imageXML->LinkEndChild(nameXML);

	TiXmlElement* filenameXML = new TiXmlElement("Filename");
	TiXmlText* filenameText = new TiXmlText(filename());
	nameXML->LinkEndChild(filenameText);
	imageXML->LinkEndChild(filenameXML);

	return(imageXML);
}

END_NAMESPACE( Aqsis )

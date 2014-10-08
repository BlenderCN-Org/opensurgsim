// This file is a part of the OpenSurgSim project.
// Copyright 2012-2013, SimQuest Solutions Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file
/// Tests for the Image class.

#include "gtest/gtest.h"

#include "SurgSim/DataStructures/Image.h"
#include "SurgSim/Math/Matrix.h"

namespace
{
double epsilon = 1e-10;
}


namespace SurgSim
{
namespace DataStructures
{

template <class T>
class ImageTests : public testing::Test
{
public:
	typedef T Scalar;
};

typedef ::testing::Types<unsigned char, char, unsigned int, int, float, double> ImageTestTypes;
TYPED_TEST_CASE(ImageTests, ImageTestTypes);

TYPED_TEST(ImageTests, Construct)
{
	typedef typename TestFixture::Scalar T;

	ASSERT_NO_THROW({Image<T> image;});
	ASSERT_NO_THROW({Image<T> image(10, 10, 1);});
	ASSERT_NO_THROW({Image<T> image(100, 10, 3);});
	ASSERT_NO_THROW({Image<T> image(512, 1024, 4);});

	T array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	ASSERT_NO_THROW({Image<T> image(3, 3, 1, array);});
}

TYPED_TEST(ImageTests, ConstructFromOtherType)
{
	typedef typename TestFixture::Scalar T;

	double array[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
	Image<T> image(3, 3, 1, array);
	for (int i = 0; i < 9; i++)
	{
		EXPECT_NEAR(static_cast<T>(array[i]), image.getData()[i], epsilon);
	}
}

TYPED_TEST(ImageTests, Copy)
{
	typedef typename TestFixture::Scalar T;
	Image<T> image(10, 10, 1);
	Image<T> newImage(image);
	EXPECT_EQ(image.getSize(), newImage.getSize());
	EXPECT_NE(image.getData(), newImage.getData());
}

TYPED_TEST(ImageTests, Assign)
{
	typedef typename TestFixture::Scalar T;
	Image<T> image(10, 10, 1);

	Image<T> newImage;
	newImage = image;

	EXPECT_EQ(image.getSize(), newImage.getSize());
	EXPECT_NE(image.getData(), newImage.getData());
}

TYPED_TEST(ImageTests, Accessors)
{
	typedef typename TestFixture::Scalar T;
	{
		Image<T> image;
		EXPECT_EQ(0, image.getWidth());
		EXPECT_EQ(0, image.getHeight());
		EXPECT_EQ(0, image.getNumChannels());
		EXPECT_EQ(nullptr, image.getData());

		std::array<size_t, 3> size = {0, 0, 0};
		EXPECT_EQ(size, image.getSize());
	}
	{
		Image<T> image(10, 20, 30);
		EXPECT_EQ(10, image.getWidth());
		EXPECT_EQ(20, image.getHeight());
		EXPECT_EQ(30, image.getNumChannels());

		std::array<size_t, 3> size = {10, 20, 30};
		EXPECT_EQ(size, image.getSize());
	}
	{
		T array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
		Image<T> image(3, 3, 1, array);
		EXPECT_EQ(3, image.getWidth());
		EXPECT_EQ(3, image.getHeight());
		EXPECT_EQ(1, image.getNumChannels());

		std::array<size_t, 3> size = {3, 3, 1};
		EXPECT_EQ(size, image.getSize());

		for (int i = 0; i < 9; i++)
		{
			EXPECT_NEAR(array[i], image.getData()[i], epsilon);
		}
	}
}

TYPED_TEST(ImageTests, Move)
{
	typedef typename TestFixture::Scalar T;

	{
		Image<T> oldImage(3, 3, 1);
		T* const dataPtr = oldImage.getData();
		Image<T> newImage = std::move(oldImage);

		EXPECT_EQ(nullptr, oldImage.getData());
		EXPECT_NE(dataPtr, oldImage.getData());
		EXPECT_EQ(0, oldImage.getWidth());
		EXPECT_EQ(0, oldImage.getHeight());
		EXPECT_EQ(0, oldImage.getNumChannels());

		EXPECT_EQ(dataPtr, newImage.getData());
		EXPECT_EQ(3, newImage.getWidth());
		EXPECT_EQ(3, newImage.getHeight());
		EXPECT_EQ(1, newImage.getNumChannels());
	}
	{
		Image<T> oldImage(15, 25, 4);
		T* const dataPtr = oldImage.getData();
		Image<T> newImage(std::move(oldImage));

		EXPECT_EQ(nullptr, oldImage.getData());
		EXPECT_EQ(0, oldImage.getWidth());
		EXPECT_EQ(0, oldImage.getHeight());
		EXPECT_EQ(0, oldImage.getNumChannels());

		EXPECT_EQ(dataPtr, newImage.getData());
		EXPECT_EQ(15, newImage.getWidth());
		EXPECT_EQ(25, newImage.getHeight());
		EXPECT_EQ(4, newImage.getNumChannels());
	}
}

TYPED_TEST(ImageTests, PointerAccess)
{
	typedef typename TestFixture::Scalar T;

	Image<T> image(3, 3, 1);
	T array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	std::copy(array, array+9, image.getData());

	for (int i = 0; i < 9; i++)
	{
		EXPECT_NEAR(array[i], image.getData()[i], epsilon);
	}
}

TYPED_TEST(ImageTests, EigenAccess)
{
	typedef typename TestFixture::Scalar T;
	typedef Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

	{
		Image<T> image(50, 1000, 2);
		EXPECT_NO_THROW(image.getChannel(0));
		EXPECT_NO_THROW(image.getChannel(1));
		EXPECT_THROW(image.getChannel(2), SurgSim::Framework::AssertionFailure);
		EXPECT_THROW(image.getChannel(100), SurgSim::Framework::AssertionFailure);
	}
	{
		Image<T> image(300, 300, 3);
		image.getChannel(0) = Matrix::Constant(300, 300, T(0));
		image.getChannel(1) = Matrix::Constant(300, 300, T(1));
		image.getChannel(2) = Matrix::Constant(300, 300, T(2));

		for (int i = 0; i < 300*300; i++)
		{
			EXPECT_NEAR(i % 3, image.getData()[i], epsilon);
		}

		Matrix total = image.getChannel(0) + image.getChannel(1) + image.getChannel(2);
		EXPECT_TRUE(total.isApprox(Matrix::Constant(300, 300, T(3))));
	}
	{
		Image<T> image(6, 6, 1);
		image.getChannel(0) <<  0,  1,  2,  3,  4,  5,
							   10, 11, 12, 13, 14, 15,
							   20, 21, 22, 23, 24, 25,
							   30, 31, 32, 33, 34, 35,
							   30, 41, 42, 43, 44, 45,
							   50, 51, 52, 53, 54, 55;
		Matrix matrix = image.getChannel(0);
		EXPECT_NEAR(24, matrix(2, 4), epsilon);
		EXPECT_NEAR(15, matrix(1, 5), epsilon);
		EXPECT_NEAR(30, matrix(3, 0), epsilon);
		EXPECT_NEAR(54, matrix(5, 4), epsilon);
	}
	{
		Image<T> image(10, 30, 1);
		image.getChannel(0) = Matrix::Constant(10, 30, T(1));
		image.getChannel(0) *= 2;
		EXPECT_TRUE(image.getChannel(0).isApprox(Matrix::Constant(10, 30, T(2))));

		image.getChannel(0) = image.getChannel(0).array() + T(3);
		EXPECT_TRUE(image.getChannel(0).isApprox(Matrix::Constant(10, 30, T(5))));
	}
}

}
}

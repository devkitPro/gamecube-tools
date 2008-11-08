/* -----------------------------------------------------------------------------

	Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the 
	"Software"), to	deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to 
	permit persons to whom the Software is furnished to do so, subject to 
	the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	
   -------------------------------------------------------------------------- */
   
#include "clusterfit.h"
#include "colourset.h"
#include "colourblock.h"
#include <cfloat>

namespace squish {

ClusterFit::ClusterFit( ColourSet const* colours, int flags ) 
  : ColourFit( colours, flags )
{
	// initialise the best error
#if SQUISH_USE_SIMD
	m_besterror = VEC4_CONST( FLT_MAX );
#else
	m_besterror = FLT_MAX;
#endif

	// initialise the metric
	bool perceptual = ( ( m_flags & kColourMetricPerceptual ) != 0 );
#if SQUISH_USE_SIMD
	if( perceptual )
		m_metric = Vec4( 0.2126f, 0.7152f, 0.0722f, 0.0f );
	else
		m_metric = VEC4_CONST( 1.0f );	
#else
	if( perceptual )
		m_metric = Vec3( 0.2126f, 0.7152f, 0.0722f );
	else
		m_metric = Vec3( 1.0f );
#endif

	// cache some values
	int const count = m_colours->GetCount();
	Vec3 const* values = m_colours->GetPoints();
	
	// get the covariance matrix
	Sym3x3 covariance = ComputeWeightedCovariance( count, values, m_colours->GetWeights() );
	
	// compute the principle component
	m_principle = ComputePrincipleComponent( covariance );
}

bool ClusterFit::ConstructOrdering( Vec3 const& axis, int iteration )
{
	// cache some values
	int const count = m_colours->GetCount();
	Vec3 const* values = m_colours->GetPoints();

	// build the list of dot products
	float dps[16];
	u8* order = ( u8* )m_order + 16*iteration;
	for( int i = 0; i < count; ++i )
	{
		dps[i] = Dot( values[i], axis );
		order[i] = ( u8 )i;
	}
		
	// stable sort using them
	for( int i = 0; i < count; ++i )
	{
		for( int j = i; j > 0 && dps[j] < dps[j - 1]; --j )
		{
			std::swap( dps[j], dps[j - 1] );
			std::swap( order[j], order[j - 1] );
		}
	}
	
	// check this ordering is unique
	for( int it = 0; it < iteration; ++it )
	{
		u8 const* prev = ( u8* )m_order + 16*it;
		bool same = true;
		for( int i = 0; i < count; ++i )
		{
			if( order[i] != prev[i] )
			{
				same = false;
				break;
			}
		}
		if( same )
			return false;
	}
	
	// copy the ordering and weight all the points
#if SQUISH_USE_SIMD
	Vec4 const* unweighted = m_colours->GetPointsSimd();
	Vec4 const* weights = m_colours->GetWeightsSimd();
	m_xxsum = VEC4_CONST( 0.0f );
#else
	Vec3 const* unweighted = m_colours->GetPoints();
	float const* weights = m_colours->GetWeights();
	m_xxsum = Vec3( 0.0f );
#endif
	for( int i = 0; i < count; ++i )
	{
		int p = order[i];
		m_unweighted[i] = unweighted[p];
		m_weights[i] = weights[p];
		m_weighted[i] = weights[p]*unweighted[p];
		m_xxsum += m_weighted[i]*m_weighted[i];
	}
	return true;
}

void ClusterFit::Compress3( void* block )
{
	// declare variables
	int const count = m_colours->GetCount();
#if SQUISH_USE_SIMD
	Vec4 beststart = VEC4_CONST( 0.0f );
	Vec4 bestend = VEC4_CONST( 0.0f );
	Vec4 besterror = VEC4_CONST( FLT_MAX );
	Vec4 const half = VEC4_CONST( 0.5f );
	Vec4 const zero = VEC4_CONST( 0.0f );
#else
	Vec3 beststart( 0.0f );
	Vec3 bestend( 0.0f );
	float besterror = FLT_MAX;
	float const half = 0.5f;
	float const zero = 0.0f;
#endif	

	// prepare an ordering using the principle axis
	ConstructOrdering( m_principle, 0 );
	
	// check all possible clusters and iterate on the total order
	u8 indices[16];
	u8 bestindices[16];
	int bestiteration = 0;
	
	// loop over iterations
	for( int iteration = 0;; )
	{
		// first cluster [0,i) is at the start
		for( int m = 0; m < count; ++m )
		{
			indices[m] = 0;
			m_alpha[m] = m_weights[m];
			m_beta[m] = zero;
		}
		for( int i = count; i >= 0; --i )
		{
			// second cluster [i,j) is half along
			for( int m = i; m < count; ++m )
			{
				indices[m] = 2;
				m_alpha[m] = m_beta[m] = half*m_weights[m];
			}		
			for( int j = count; j >= i; --j )
			{
				// last cluster [j,k) is at the end
				if( j < count )
				{
					indices[j] = 1;
					m_alpha[j] = zero;
					m_beta[j] = m_weights[j];
				}		
				
				// solve a least squares problem to place the endpoints
#if SQUISH_USE_SIMD
				Vec4 start, end;
				Vec4 error = SolveLeastSquares( start, end );
#else
				Vec3 start, end;
				float error = SolveLeastSquares( start, end );
#endif

				// keep the solution if it wins
#if SQUISH_USE_SIMD
				if( CompareAnyLessThan( error, besterror ) )
#else
				if( error < besterror )
#endif
				{
					beststart = start;
					bestend = end;
					for( int m = 0; m < 16; ++m )	// TODO: make this faster?
						bestindices[m] = indices[m];
					besterror = error;
					bestiteration = iteration;
				}
			}
		}
		
		// stop if we didn't improve in this iteration
		if( bestiteration != iteration )
			break;
			
		// advance if possible
		++iteration;
		if( iteration == kMaxIterations )
			break;
			
		// stop if a new iteration is an ordering that has already been tried
#if SQUISH_USE_SIMD
		Vec3 axis = ( bestend - beststart ).GetVec3();
#else
		Vec3 axis = bestend - beststart;
#endif
		if( !ConstructOrdering( axis, iteration ) )
			break;
	}
		
	// save the block if necessary
#if SQUISH_USE_SIMD
	if( CompareAnyLessThan( besterror, m_besterror ) )
#else
	if( besterror < m_besterror )
#endif
	{
		// remap the indices
		u8 const* order = ( u8* )m_order + 16*bestiteration;
		u8 unordered[16];
		for( int i = 0; i < count; ++i )
			unordered[order[i]] = bestindices[i];
		m_colours->RemapIndices( unordered, bestindices );
		
		// save the block
#if SQUISH_USE_SIMD
		WriteColourBlock3( beststart.GetVec3(), bestend.GetVec3(), bestindices, block );
#else
		WriteColourBlock3( beststart, bestend, bestindices, block );
#endif

		// save the error
		m_besterror = besterror;
	}
}

void ClusterFit::Compress4( void* block )
{
	// declare variables
	int const count = m_colours->GetCount();
#if SQUISH_USE_SIMD
	Vec4 beststart = VEC4_CONST( 0.0f );
	Vec4 bestend = VEC4_CONST( 0.0f );
	Vec4 besterror = m_besterror;
	Vec4 const twothirds = VEC4_CONST( 2.0f/3.0f );
	Vec4 const onethird = VEC4_CONST( 1.0f/3.0f );
	Vec4 const zero = VEC4_CONST( 0.0f );
#else
	Vec3 beststart( 0.0f );
	Vec3 bestend( 0.0f );
	float besterror = m_besterror;
	float const twothirds = 2.0f/3.0f;
	float const onethird = 1.0f/3.0f;
	float const zero = 0.0f;
#endif	

	// prepare an ordering using the principle axis
	ConstructOrdering( m_principle, 0 );
	
	// check all possible clusters and iterate on the total order
	u8 indices[16];
	u8 bestindices[16];
	int bestiteration = 0;
	
	// loop over iterations
	for( int iteration = 0;; )
	{
		// first cluster [0,i) is at the start
		for( int m = 0; m < count; ++m )
		{
			indices[m] = 0;
			m_alpha[m] = m_weights[m];
			m_beta[m] = zero;
		}
		for( int i = count; i >= 0; --i )
		{
			// second cluster [i,j) is one third along
			for( int m = i; m < count; ++m )
			{
				indices[m] = 2;
				m_alpha[m] = twothirds*m_weights[m];
				m_beta[m] = onethird*m_weights[m];
			}		
			for( int j = count; j >= i; --j )
			{
				// third cluster [j,k) is two thirds along
				for( int m = j; m < count; ++m )
				{
					indices[m] = 3;
					m_alpha[m] = onethird*m_weights[m];
					m_beta[m] = twothirds*m_weights[m];
				}		
				for( int k = count; k >= j; --k )
				{
					// last cluster [k,n) is at the end
					if( k < count )
					{
						indices[k] = 1;
						m_alpha[k] = zero;
						m_beta[k] = m_weights[k];
					}				
					
					// solve a least squares problem to place the endpoints
#if SQUISH_USE_SIMD
					Vec4 start, end;
					Vec4 error = SolveLeastSquares( start, end );
#else
					Vec3 start, end;
					float error = SolveLeastSquares( start, end );
#endif

					// keep the solution if it wins
#if SQUISH_USE_SIMD
					if( CompareAnyLessThan( error, besterror ) )
#else
					if( error < besterror )
#endif
					{
						beststart = start;
						bestend = end;
						for( int m = 0; m < 16; ++m )	// TODO: make this faster?
							bestindices[m] = indices[m];	
						besterror = error;
						bestiteration = iteration;
					}
				}
			}
		}
		
		// stop if we didn't improve in this iteration
		if( bestiteration != iteration )
			break;
			
		// advance if possible
		++iteration;
		if( iteration == kMaxIterations )
			break;
			
		// stop if a new iteration is an ordering that has already been tried
#if SQUISH_USE_SIMD
		Vec3 axis = ( bestend - beststart ).GetVec3();
#else
		Vec3 axis = bestend - beststart;
#endif
		if( !ConstructOrdering( axis, iteration ) )
			break;
	}

	// save the block if necessary
#if SQUISH_USE_SIMD
	if( CompareAnyLessThan( besterror, m_besterror ) )
#else
	if( besterror < m_besterror )
#endif
	{
		// remap the indices
		u8 const* order = ( u8* )m_order + 16*bestiteration;
		u8 unordered[16];
		for( int i = 0; i < count; ++i )
			unordered[order[i]] = bestindices[i];
		m_colours->RemapIndices( unordered, bestindices );
		
		// save the block
#if SQUISH_USE_SIMD
		WriteColourBlock4( beststart.GetVec3(), bestend.GetVec3(), bestindices, block );
#else
		WriteColourBlock4( beststart, bestend, bestindices, block );
#endif

		// save the error
		m_besterror = besterror;
	}
}

#if SQUISH_USE_SIMD
Vec4 ClusterFit::SolveLeastSquares( Vec4& start, Vec4& end ) const
{
	// accumulate all the quantities we need
	int const count = m_colours->GetCount();
	Vec4 alpha2_sum = VEC4_CONST( 0.0f );
	Vec4 beta2_sum = VEC4_CONST( 0.0f );
	Vec4 alphabeta_sum = VEC4_CONST( 0.0f );
	Vec4 alphax_sum = VEC4_CONST( 0.0f );
	Vec4 betax_sum = VEC4_CONST( 0.0f );
	for( int i = 0; i < count; ++i )
	{
		Vec4 alpha = m_alpha[i];
		Vec4 beta = m_beta[i];
		Vec4 x = m_weighted[i];
	
		alpha2_sum = MultiplyAdd( alpha, alpha, alpha2_sum );
		beta2_sum = MultiplyAdd( beta, beta, beta2_sum );
		alphabeta_sum = MultiplyAdd( alpha, beta, alphabeta_sum );
		alphax_sum = MultiplyAdd( alpha, x, alphax_sum );
		betax_sum = MultiplyAdd( beta, x, betax_sum );	
	}

	// select the results
	Vec4 const zero = VEC4_CONST( 0.0f );
	Vec4 beta2_sum_zero = CompareEqual( beta2_sum, zero );
	Vec4 alpha2_sum_zero = CompareEqual( alpha2_sum, zero );
	
	Vec4 a1 = alphax_sum*Reciprocal( alpha2_sum );
	Vec4 b1 = betax_sum*Reciprocal( beta2_sum );
	
	Vec4 factor = Reciprocal( NegativeMultiplySubtract( 
		alphabeta_sum, alphabeta_sum, alpha2_sum*beta2_sum 
	) );
	Vec4 a2 = NegativeMultiplySubtract( 
		betax_sum, alphabeta_sum, alphax_sum*beta2_sum
	)*factor;
	Vec4 b2 = NegativeMultiplySubtract(
		alphax_sum, alphabeta_sum, betax_sum*alpha2_sum
	)*factor;
	
	Vec4 a = Select( Select( a2, a1, beta2_sum_zero ), zero, alpha2_sum_zero );
	Vec4 b = Select( Select( b2, b1, alpha2_sum_zero ), zero, beta2_sum_zero );

	// clamp the output to [0, 1]
	Vec4 const one = VEC4_CONST( 1.0f );
	Vec4 const half = VEC4_CONST( 0.5f );
	a = Min( one, Max( zero, a ) );
	b = Min( one, Max( zero, b ) );

	// clamp to the grid
	Vec4 const grid( 31.0f, 63.0f, 31.0f, 0.0f );
	Vec4 const gridrcp( 1.0f/31.0f, 1.0f/63.0f, 1.0f/31.0f, 0.0f );
	Vec4 const onethird = VEC4_CONST( 1.0f/3.0f );
	Vec4 const twothirds = VEC4_CONST( 2.0f/3.0f );
	a = Truncate( MultiplyAdd( grid, a, half ) )*gridrcp;
	b = Truncate( MultiplyAdd( grid, b, half ) )*gridrcp;
	
	// compute the error
	Vec4 const two = VEC4_CONST( 2.0 );
	Vec4 e1 = MultiplyAdd( b*b, beta2_sum, m_xxsum );
	Vec4 e2 = MultiplyAdd( a, alphax_sum, b*betax_sum );
	Vec4 e3 = MultiplyAdd( a*a, alpha2_sum, e1 );
	Vec4 e4 = MultiplyAdd( a*b*alphabeta_sum - e2, two, e3 );

	// apply the metric to the error term
	Vec4 e5 = e4*m_metric;
	Vec4 error = e5.SplatX() + e5.SplatY() + e5.SplatZ();
	
	// save the start and end
	start = a;
	end = b;
	return error;
}
#else
float ClusterFit::SolveLeastSquares( Vec3& start, Vec3& end ) const
{
	// accumulate all the quantities we need
	int const count = m_colours->GetCount();
	float alpha2_sum = 0.0f;
	float beta2_sum = 0.0f;
	float alphabeta_sum = 0.0f;
	Vec3 alphax_sum( 0.0f );
	Vec3 betax_sum( 0.0f );	
	for( int i = 0; i < count; ++i )
	{
		float alpha = m_alpha[i];
		float beta = m_beta[i];
		Vec3 const& x = m_weighted[i];
		
		alpha2_sum += alpha*alpha;
		beta2_sum += beta*beta;
		alphabeta_sum += alpha*beta;
		alphax_sum += alpha*x;
		betax_sum += beta*x;
	}

	// zero where non-determinate
	Vec3 a, b;
	if( beta2_sum == 0.0f )
	{
		a = alphax_sum/alpha2_sum;
		b = Vec3( 0.0f );
	}
	else if( alpha2_sum == 0.0f )
	{
		a = Vec3( 0.0f );
		b = betax_sum/beta2_sum;
	}
	else
	{
		float factor = 1.0f/( alpha2_sum*beta2_sum - alphabeta_sum*alphabeta_sum );
		
		a = ( alphax_sum*beta2_sum - betax_sum*alphabeta_sum )*factor;
		b = ( betax_sum*alpha2_sum - alphax_sum*alphabeta_sum )*factor;
	}
	
	// clamp the output to [0, 1]
	Vec3 const one( 1.0f );
	Vec3 const zero( 0.0f );
	a = Min( one, Max( zero, a ) );
	b = Min( one, Max( zero, b ) );

	// clamp to the grid
	Vec3 const grid( 31.0f, 63.0f, 31.0f );
	Vec3 const gridrcp( 1.0f/31.0f, 1.0f/63.0f, 1.0f/31.0f );
	Vec3 const half( 0.5f );
	a = Truncate( grid*a + half )*gridrcp;
	b = Truncate( grid*b + half )*gridrcp;

	// compute the error
	Vec3 e1 = a*a*alpha2_sum + b*b*beta2_sum + m_xxsum
		+ 2.0f*( a*b*alphabeta_sum - a*alphax_sum - b*betax_sum );

	// apply the metric to the error term
	float error = Dot( e1, m_metric );
	
	// save the start and end
	start = a;
	end = b;
	return error;
}
#endif

} // namespace squish

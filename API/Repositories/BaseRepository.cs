using FlagGuessingGameAPI;

namespace H2Projekt.Infrastructure.Repositories
{
    public class BaseRepository : IBaseRepository
    {
        protected readonly AppDbContext _appDbContext;

        public BaseRepository(AppDbContext appDbContext)
        {
            _appDbContext = appDbContext;
        }

        public async Task<int> SaveChangesAsync(CancellationToken cancellationToken)
        {
            return await _appDbContext.SaveChangesAsync(cancellationToken);
        }
    }
}

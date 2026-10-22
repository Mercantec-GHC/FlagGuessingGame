namespace H2Projekt.Infrastructure.Repositories
{
    public interface IBaseRepository
    {
        Task<int> SaveChangesAsync(CancellationToken cancellationToken);
    }
}
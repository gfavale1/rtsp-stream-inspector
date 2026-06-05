export default function SectionCard({ title, description, children, actions }) {
  return (
    <section className="section-card">
      <div className="section-header">
        <div>
          <h2>{title}</h2>
          {description && <p>{description}</p>}
        </div>
        {actions && <div className="section-actions">{actions}</div>}
      </div>
      {children}
    </section>
  );
}
